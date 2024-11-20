#include "FrustumCull.hpp"

#include "Core/GPUFramework/Vulkan/ComputePipeline.hpp"

FrustumCullPass::FrustumCullPass(const GPUContext* gpuContext, ResourceManager* resourceManager, const Scene* scene)
	:gpuContext{ gpuContext }
	, resourceManager{ resourceManager }
	, scene{ scene } 
{
}

FrustumCullPass::~FrustumCullPass()
{
	delete computePipeline;
	delete pipelineLayout;
	delete descriptorSet;
	delete descriptorSetLayout;
}

void FrustumCullPass::prepare()
{
	meshInfos.reserve(scene->getMeshCount());
	for (int i = 0; i < scene->getMeshCount(); i++) {
		MeshInfo info{};
		info.meshIndex = i;
		info.firstIndex = scene->indexOffsets[i];
		info.indexCount = scene->getMeshes()[i]->getIndices().size();
		info.vertexOffset = scene->vertexOffsets[i];
		info.minAABB = scene->getMeshes()[i]->getAABB().min;
		info.maxAABB = scene->getMeshes()[i]->getAABB().max;
		meshInfos.push_back(std::move(info));
	}

	auto meshCount = scene->getMeshCount();
	meshBuffer = resourceManager->createBuffer(
		sizeof(MeshInfo) * meshCount, 
		vk::BufferUsageFlagBits::eStorageBuffer);
	indirectDrawCommandBuffer = resourceManager->createIndirectBuffer(
		sizeof(MeshDrawCommand) * meshCount,
		vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
		VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY, false);
	uniformBuffer = resourceManager->createBuffer(
		sizeof(Uniform) * meshCount, 
		vk::BufferUsageFlagBits::eUniformBuffer);

	std::vector<vk::DescriptorSetLayoutBinding> bindings;
	bindings.push_back(vk::DescriptorSetLayoutBinding{ 0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute });
	bindings.push_back(vk::DescriptorSetLayoutBinding{ 1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute });
	bindings.push_back(vk::DescriptorSetLayoutBinding{ 2, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eCompute });
	descriptorSetLayout = gpuContext->createDescriptorSetLayout(0, bindings);

	constants.stageFlags = vk::ShaderStageFlagBits::eCompute;
	constants.size = sizeof(CullData);
	constants.offset = 0;
	pipelineLayout = new PipelineLayout{ *gpuContext->getDevice(), {descriptorSetLayout->getHandle()}, {constants} };
	const ShaderModule* cullShader = resourceManager->findShader("frustumcull.comp");
	computePipeline = new ComputePipeline{ *gpuContext->getDevice() , pipelineLayout, VK_NULL_HANDLE, cullShader };

	std::unordered_map<uint32_t, vk::DescriptorBufferInfo> bufferInfos;
	bufferInfos[0] = vk::DescriptorBufferInfo{ meshBuffer->getHandle(), 0, sizeof(MeshInfo) * meshInfos.size() };
	bufferInfos[1] = vk::DescriptorBufferInfo{ indirectDrawCommandBuffer->getHandle(), 0, sizeof(MeshDrawCommand) * meshInfos.size() };
	bufferInfos[2] = vk::DescriptorBufferInfo{ uniformBuffer->getHandle(), 0, sizeof(Uniform) * meshInfos.size() };

	descriptorSet = gpuContext->requireDescriptorSet(descriptorSetLayout, bufferInfos, {});

	meshBuffer->copyToGPU(static_cast<const void*>(meshInfos.data()), sizeof(MeshInfo) * meshInfos.size());
}

void FrustumCullPass::update(uint32_t frameIndex)
{
	uniforms.clear();
	auto models = scene->getTransforms();
	uniforms.reserve(scene->getMeshCount());
	for (int i = 0; i < scene->getMeshCount(); i++) {
		uniforms.push_back(Uniform(models[i]));
	}
	uniformBuffer->copyToGPU(static_cast<const void*>(uniforms.data()), sizeof(Uniform) * uniforms.size());
}

void FrustumCullPass::record(vk::CommandBuffer commandBuffer)
{
	gpuContext->bufferBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits2::eHost, vk::PipelineStageFlagBits2::eComputeShader,
		vk::AccessFlagBits2::eHostWrite, vk::AccessFlagBits2::eShaderRead,
		meshBuffer, 0, meshBuffer->getSize());

	gpuContext->bufferBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits2::eNone, vk::PipelineStageFlagBits2::eComputeShader,
		vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eShaderWrite,
		indirectDrawCommandBuffer, 0, indirectDrawCommandBuffer->getSize());

	gpuContext->bufferBarrier(
		commandBuffer,
		vk::PipelineStageFlagBits2::eHost, vk::PipelineStageFlagBits2::eComputeShader,
		vk::AccessFlagBits2::eHostWrite, vk::AccessFlagBits2::eShaderRead,
		uniformBuffer, 0, uniformBuffer->getSize());

	auto normalizePlane = [](Vec4 p)->Vec4
		{
			return p / glm::length(Vec3(p));
		};
	auto camera = scene->getCamera();
	auto constant = CullData{};
	constant.projectionView = glm::transpose(camera->getProjectionMatrix() * camera->getViewMatrix());
	constant.P00 = camera->getProjectionMatrix()[0][0];
	constant.P11 = camera->getProjectionMatrix()[1][1];
	constant.zNear = camera->getNear();
	constant.zFar = camera->getFar();
	auto projection = camera->getProjectionMatrix();
	auto projectionT = glm::transpose(projection);
	Vec4 frustumX = normalizePlane(projectionT[3] + projectionT[0]); // x + w < 0
	Vec4 frustumY = normalizePlane(projectionT[3] + projectionT[1]); // y + w < 0
	constant.frustum[0] = frustumX.x;
	constant.frustum[1] = frustumX.z;
	constant.frustum[2] = frustumY.y;
	constant.frustum[3] = frustumY.z;
	constant.model_count = scene->getMeshCount();

	commandBuffer.pushConstants<CullData>(
		pipelineLayout->getHandle(),
		vk::ShaderStageFlagBits::eCompute, 0,
		{ constant });

	commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, computePipeline->getHandle());
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipelineLayout->getHandle(), 0, { descriptorSet->getHandle() }, {});
	commandBuffer.dispatch(static_cast<uint32_t>((scene->getMeshCount()+63)/64), 1, 1);
}