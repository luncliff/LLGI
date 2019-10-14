
#pragma once

#include "LLGI.Base.h"

namespace LLGI
{
static constexpr int NumTexture = 8;

class VertexBuffer;
class IndexBuffer;

/**
	@brief	command list
	@note
	CommandList has a swap buffer. So you don't need to consider buffering.
	CommandList has references related to rendering.
	References are released after the number of swapCount has passed.

	Limitation :
	Begin and End are need to call only once in one frame.
	Command list must not be released after finishing rendering.
*/
class CommandList : public ReferenceObject
{
protected:
	struct BindingVertexBuffer
	{
		VertexBuffer* vertexBuffer = nullptr;
		int32_t stride = 0;
		int32_t offset = 0;
	};

	struct BindingIndexBuffer
	{
		IndexBuffer* indexBuffer = nullptr;
		int32_t offset = 0;
	};

	struct BindingTexture
	{
		Texture* texture = nullptr;
		TextureWrapMode wrapMode = TextureWrapMode::Clamp;
		TextureMinMagFilter minMagFilter = TextureMinMagFilter::Nearest;
	};

private:
	struct SwapObject
	{
		std::vector<ReferenceObject*> referencedObjects;
	};

	int32_t swapIndex_ = -1;
	int32_t swapCount_ = 0;
	std::vector<SwapObject> swapObjects;

	BindingVertexBuffer bindingVertexBuffer;
	BindingIndexBuffer bindingIndexBuffer;

	PipelineState* currentPipelineState = nullptr;

	bool isVertexBufferDirtied = true;
	bool isCurrentIndexBufferDirtied = true;
	bool isPipelineDirtied = true;

	std::array<ConstantBuffer*, static_cast<int>(ShaderStageType::Max)> constantBuffers;

protected:
	std::array<std::array<BindingTexture, NumTexture>, static_cast<int>(ShaderStageType::Max)> currentTextures;

protected:
	void GetCurrentVertexBuffer(BindingVertexBuffer& buffer, bool& isDirtied);
	void GetCurrentIndexBuffer(BindingIndexBuffer& buffer, bool& isDirtied);
	void GetCurrentPipelineState(PipelineState*& pipelineState, bool& isDirtied);
	void GetCurrentConstantBuffer(ShaderStageType type, ConstantBuffer*& buffer);
	void RegisterReferencedObject(ReferenceObject* referencedObject);

public:
	CommandList(int32_t swapCount = 3);
	virtual ~CommandList();

	/**
	@brief
	added a command into supecified context.
	This function can be called once by a frame.
	*/
	virtual void Begin();

	/**
		@brief
		added a command into supecified context. This function is supported in some platform.
		Internal context is not used if platformContextPtr is null in Metal.
		This function can be called once by a frame.
	*/
	virtual bool BeginWithPlatform(void* platformContextPtr);

	virtual void End();

	virtual void SetScissor(int32_t x, int32_t y, int32_t width, int32_t height);
	virtual void Draw(int32_t pritimiveCount);
	virtual void SetVertexBuffer(VertexBuffer* vertexBuffer, int32_t stride, int32_t offset);
	virtual void SetIndexBuffer(IndexBuffer* indexBuffer, int32_t offset = 0);
	virtual void SetPipelineState(PipelineState* pipelineState);
	virtual void SetConstantBuffer(ConstantBuffer* constantBuffer, ShaderStageType shaderStage);
	virtual void
	SetTexture(Texture* texture, TextureWrapMode wrapMode, TextureMinMagFilter minmagFilter, int32_t unit, ShaderStageType shaderStage);
	virtual void BeginRenderPass(RenderPass* renderPass);

	/**
		@brief
		added a command into supecified renderpass. This function is supported in some platform.
	*/
	virtual bool BeginRenderPassWithPlatformPtr(void* platformPtr);

	virtual void EndRenderPass() {}

	/**
		@brief
		The pair of BeginRenderPassWithPlatformPtr
	*/
	virtual bool EndRenderPassWithPlatformPtr() { return false; }

	/**
		@brief	send a memory in specified VertexBuffer from cpu to gpu
	*/
	virtual void SetData(VertexBuffer* vertexBuffer, int32_t offset, int32_t size, const void* data);

	/**
		@brief	send a memory in specified IndexBuffer from cpu to gpu
	*/
	virtual void SetData(IndexBuffer* indexBuffer, int32_t offset, int32_t size, const void* data);

	/**
		@brief	send a memory in specified ConstantBuffer from cpu to gpu
	*/
	virtual void SetData(ConstantBuffer* constantBuffer, int32_t offset, int32_t size, const void* data);

	/**
		@brief	send a memory in specified texture from cpu to gpu
	*/
	virtual void SetImageData2D(Texture* texture, int32_t x, int32_t y, int32_t width, int32_t height, const void* data);

	/**
		@brief wait until this command is completed.
	*/
	virtual void WaitUntilCompleted();
};

// TODO: see https://github.com/altseed/LLGI/issues/26
class CommandListPool : public ReferenceObject
{
public:
	CommandListPool(int32_t swapCount);
	virtual ~CommandListPool() = default;

	virtual CommandList* Get();
};

} // namespace LLGI
