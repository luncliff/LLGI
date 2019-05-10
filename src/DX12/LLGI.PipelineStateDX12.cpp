#pragma once

#include "LLGI.PipelineStateDX12.h"
#include "../LLGI.PipelineState.h"
#include "LLGI.ShaderDX12.h"

namespace LLGI
{

PipelineStateDX12::PipelineStateDX12(GraphicsDX12* graphics)
{
	SafeAddRef(graphics);
	graphics_ = CreateSharedPtr(graphics);
}

PipelineStateDX12::~PipelineStateDX12()
{
	for (auto& shader : shaders)
	{
		SafeRelease(shader);
	}
}

void PipelineStateDX12::SetShader(ShaderStageType stage, Shader* shader)
{
	SafeAddRef(shader);
	SafeRelease(shaders[static_cast<int>(stage)]);
	shaders[static_cast<int>(stage)] = shader;
}

void PipelineStateDX12::Compile()
{
	CreateRootSignature();

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc = {};

	for (size_t i = 0; i < shaders.size(); i++)
	{
		auto shader = static_cast<ShaderDX12*>(shaders[i])->GetData();

		if (i == static_cast<int>(ShaderStageType::Pixel))
		{
			pipelineStateDesc.PS.pShaderBytecode = shader.data();
			pipelineStateDesc.PS.BytecodeLength = shader.size();
		}
		else if (i == static_cast<int>(ShaderStageType::Vertex))
		{
			pipelineStateDesc.VS.pShaderBytecode = shader.data();
			pipelineStateDesc.VS.BytecodeLength = shader.size();
		}
	}

	// setup a vertex layout
	std::array<D3D12_INPUT_ELEMENT_DESC, 16> elementDescs;
	elementDescs.fill(D3D12_INPUT_ELEMENT_DESC{});
	int32_t elementOffset = 0;

	for (int i = 0; i < VertexLayoutCount; i++)
	{
		elementDescs[i].SemanticName = this->VertexLayoutNames[i].c_str();
		elementDescs[i].SemanticIndex = 0;
		elementDescs[i].AlignedByteOffset = elementOffset;
		elementDescs[i].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

		if (VertexLayouts[i] == VertexLayoutFormat::R32G32_FLOAT)
		{
			elementDescs[i].Format = DXGI_FORMAT_R32G32_FLOAT;
			elementOffset += sizeof(float) * 2;
		}

		if (VertexLayouts[i] == VertexLayoutFormat::R32G32B32_FLOAT)
		{
			elementDescs[i].Format = DXGI_FORMAT_R32G32B32_FLOAT;
			elementOffset += sizeof(float) * 3;
		}

		if (VertexLayouts[i] == VertexLayoutFormat::R32G32B32A32_FLOAT)
		{
			elementDescs[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			elementOffset += sizeof(float) * 4;
		}

		if (VertexLayouts[i] == VertexLayoutFormat::R8G8B8A8_UNORM)
		{
			elementDescs[i].Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			elementOffset += sizeof(float) * 1;
		}

		if (VertexLayouts[i] == VertexLayoutFormat::R8G8B8A8_UINT)
		{
			elementDescs[i].Format = DXGI_FORMAT_R8G8B8A8_UINT;
			elementOffset += sizeof(float) * 1;
		}
	}

	// setup a topology
	if (Topology == TopologyType::Triangle)
		pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	if (Topology == TopologyType::Line)
		pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;


	// TODO...(generate from parameters)
	D3D12_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	if (Culling == CullingMode::Clockwise)
		rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	if (Culling == CullingMode::CounterClockwise)
		rasterizerDesc.CullMode = D3D12_CULL_MODE_FRONT;
	if (Culling == CullingMode::DoubleSide)
		rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;

	rasterizerDesc.FrontCounterClockwise = TRUE;
	rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	// setup render target blend
	D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {};
	if (IsBlendEnabled)
	{
		renderTargetBlendDesc.BlendEnable = TRUE;

		std::array<D3D12_BLEND_OP, 10> blendOps;
		blendOps[static_cast<int>(BlendEquationType::Add)] = D3D12_BLEND_OP_ADD;
		blendOps[static_cast<int>(BlendEquationType::Sub)] = D3D12_BLEND_OP_SUBTRACT;
		blendOps[static_cast<int>(BlendEquationType::ReverseSub)] = D3D12_BLEND_OP_REV_SUBTRACT;
		blendOps[static_cast<int>(BlendEquationType::Min)] = D3D12_BLEND_OP_MIN;
		blendOps[static_cast<int>(BlendEquationType::Max)] = D3D12_BLEND_OP_MAX;

		std::array<D3D12_BLEND, 20> blendFuncs;
		blendFuncs[static_cast<int>(BlendFuncType::Zero)] = D3D12_BLEND_ZERO;
		blendFuncs[static_cast<int>(BlendFuncType::One)] = D3D12_BLEND_ONE;
		blendFuncs[static_cast<int>(BlendFuncType::SrcColor)] = D3D12_BLEND_SRC_COLOR;
		blendFuncs[static_cast<int>(BlendFuncType::OneMinusSrcColor)] = D3D12_BLEND_INV_SRC_COLOR;
		blendFuncs[static_cast<int>(BlendFuncType::SrcAlpha)] = D3D12_BLEND_SRC_ALPHA;
		blendFuncs[static_cast<int>(BlendFuncType::OneMinusSrcAlpha)] = D3D12_BLEND_INV_SRC_ALPHA;
		blendFuncs[static_cast<int>(BlendFuncType::DstColor)] = D3D12_BLEND_DEST_COLOR;
		blendFuncs[static_cast<int>(BlendFuncType::OneMinusDstColor)] = D3D12_BLEND_INV_DEST_COLOR;
		blendFuncs[static_cast<int>(BlendFuncType::DstAlpha)] = D3D12_BLEND_DEST_ALPHA;
		blendFuncs[static_cast<int>(BlendFuncType::OneMinusDstAlpha)] = D3D12_BLEND_INV_DEST_ALPHA;

		renderTargetBlendDesc.SrcBlend = blendFuncs[static_cast<int>(BlendSrcFunc)];
		renderTargetBlendDesc.DestBlend = blendFuncs[static_cast<int>(BlendDstFunc)];
		renderTargetBlendDesc.SrcBlendAlpha = blendFuncs[static_cast<int>(BlendSrcFuncAlpha)];
		renderTargetBlendDesc.DestBlendAlpha = blendFuncs[static_cast<int>(BlendDstFuncAlpha)];
		renderTargetBlendDesc.BlendOp = blendOps[static_cast<int>(BlendEquationRGB)];
		renderTargetBlendDesc.BlendOpAlpha = blendOps[static_cast<int>(BlendEquationAlpha)];

		// TODO:
		renderTargetBlendDesc.LogicOpEnable = FALSE;
		renderTargetBlendDesc.LogicOp = D3D12_LOGIC_OP_COPY;
	}
	else
		renderTargetBlendDesc.BlendEnable = FALSE;

	// setup a blend state
	D3D12_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
	{
		blendDesc.RenderTarget[i] = renderTargetBlendDesc;
	}

	pipelineStateDesc.InputLayout.pInputElementDescs = elementDescs.data();
	pipelineStateDesc.InputLayout.NumElements = VertexLayoutCount;
	pipelineStateDesc.pRootSignature = RootSignature_;
	pipelineStateDesc.RasterizerState = rasterizerDesc;
	pipelineStateDesc.BlendState = blendDesc;

	// setup a depth stencil
	std::array<D3D12_COMPARISON_FUNC, 10> depthCompareOps;
	depthCompareOps[static_cast<int>(DepthFuncType::Never)] = D3D12_COMPARISON_FUNC_NEVER;
	depthCompareOps[static_cast<int>(DepthFuncType::Less)] = D3D12_COMPARISON_FUNC_LESS;
	depthCompareOps[static_cast<int>(DepthFuncType::Equal)] = D3D12_COMPARISON_FUNC_EQUAL;
	depthCompareOps[static_cast<int>(DepthFuncType::LessEqual)] = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	depthCompareOps[static_cast<int>(DepthFuncType::Greater)] = D3D12_COMPARISON_FUNC_GREATER;
	depthCompareOps[static_cast<int>(DepthFuncType::NotEqual)] = D3D12_COMPARISON_FUNC_NOT_EQUAL;
	depthCompareOps[static_cast<int>(DepthFuncType::GreaterEqual)] = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
	depthCompareOps[static_cast<int>(DepthFuncType::Always)] = D3D12_COMPARISON_FUNC_ALWAYS;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthFunc = depthCompareOps[static_cast<int>(DepthFunc)];
	depthStencilDesc.DepthEnable = IsDepthTestEnabled;
	depthStencilDesc.StencilEnable = IsDepthTestEnabled;
	// TODO

	// TODO (from renderpass)
	pipelineStateDesc.NumRenderTargets = 1;
	pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	pipelineStateDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	pipelineStateDesc.SampleDesc.Count = 1;

	auto hr = graphics_->GetDevice()->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(&pipelineState_));
	SafeAddRef(pipelineState_);

	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}

	return;

FAILED_EXIT:
	SafeRelease(pipelineState_);
	return;
}

bool PipelineStateDX12::CreateRootSignature()
{
	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.NumParameters = 0;
	desc.pParameters = nullptr;
	desc.NumStaticSamplers = 0;
	desc.pStaticSamplers = nullptr;
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	auto hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &Signature_, nullptr);
	SafeAddRef(Signature_);
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
	}

	hr = graphics_->GetDevice()->CreateRootSignature(
		0, Signature_->GetBufferPointer(), Signature_->GetBufferSize(), IID_PPV_ARGS(&RootSignature_));
	if (FAILED(hr))
	{
		goto FAILED_EXIT;
		SafeRelease(Signature_);
	}
	SafeAddRef(RootSignature_);
	return true;

FAILED_EXIT:
	SafeRelease(RootSignature_);
	return false;
}

} // namespace LLGI
