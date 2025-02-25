
#pragma once

#include "../LLGI.PipelineState.h"
#include "LLGI.BaseVulkan.h"
#include "LLGI.GraphicsVulkan.h"

namespace LLGI
{

class PipelineStateVulkan : public PipelineState
{
private:
	GraphicsVulkan* graphics_ = nullptr;
	std::array<Shader*, static_cast<int>(ShaderStageType::Max)> shaders;

	vk::Pipeline pipeline_ = nullptr;
	vk::PipelineLayout pipelineLayout_ = nullptr;
	std::array<vk::DescriptorSetLayout, 3> descriptorSetLayouts_;

	vk::Pipeline computePipeline_ = nullptr;
	vk::PipelineLayout computePipelineLayout_ = nullptr;
	std::array<vk::DescriptorSetLayout, 3> computeDescriptorSetLayouts_;

	bool CreateGraphicsPipeline();
	bool CreateComputePipeline();

public:
	PipelineStateVulkan();
	~PipelineStateVulkan() override;

	bool Initialize(GraphicsVulkan* graphics);

	void SetShader(ShaderStageType stage, Shader* shader) override;

	bool Compile() override;

	vk::Pipeline GetPipeline() const { return pipeline_; }

	vk::PipelineLayout GetPipelineLayout() const { return pipelineLayout_; }

	const std::array<vk::DescriptorSetLayout, 3>& GetDescriptorSetLayout() const { return descriptorSetLayouts_; }

	vk::Pipeline GetComputePipeline() const { return computePipeline_; }

	vk::PipelineLayout GetComputePipelineLayout() const { return computePipelineLayout_; }

	const std::array<vk::DescriptorSetLayout, 3>& GetComputeDescriptorSetLayout() const { return computeDescriptorSetLayouts_; }
};

} // namespace LLGI
