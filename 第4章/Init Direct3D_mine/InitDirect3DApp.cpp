#include "../../Common/d3dApp.h"
#include <DirectXColors.h>

using namespace DirectX;

class InitDirect3DApp : public D3DApp
{
public:
	InitDirect3DApp(HINSTANCE hInstance);
	~InitDirect3DApp();

	virtual bool Initialize()override;

private:
	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;

};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// 为调试版本开启运行时内存检测,方便监督内存泄露的情况
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		InitDirect3DApp theApp(hInstance);
		if (!theApp.Initialize())
			return 0;

		return theApp.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}

InitDirect3DApp::InitDirect3DApp(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
}

InitDirect3DApp::~InitDirect3DApp()
{
}

bool InitDirect3DApp::Initialize()
{
	if (!D3DApp::Initialize())
		return false;

	return true;
}

void InitDirect3DApp::OnResize()
{
	D3DApp::OnResize();
}

void InitDirect3DApp::Update(const GameTimer& gt)
{

}

void InitDirect3DApp::Draw(const GameTimer& gt)
{
	// 重复使用记录命令的相关内存
	//只有当与GPU 关联的命令列表执行完成时，我们才能将其重置
	ThrowIfFailed(mDirectCmdListAlloc->Reset());

	// 在通过ExecuteCommandList方法将某个命令列表加入命令队列后，
	//我们便可以重置该命令列表。以此来复用命令列表及其内存
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	// 对资源的状态进行转换，将资源从呈现状态转换为渲染目标状态
	auto m = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	mCommandList->ResourceBarrier(1, &m);

	// 设置视口和裁剪矩形。它们需要随着命令列表的重置而重置
	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// 清除后台缓冲区和深度缓冲区
	//ClearRenderTargetview方法会将指定的渲染目标清理为给定的颜色，ClearDepthStencilview方法则用于清理指定的深度/模板缓冲区。在每帧为了刷新场景而开始绘制之前，我们总是要清除后台缓冲区渲染目标和深度/模板缓冲区。
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// 指定将要渲染的缓冲区
    //ID3D12GraphicsCommandList: : OMSetRenderTargets，通过此方法即可设置我们希望在渲染流水线上使用的渲染目标和深度/模板缓冲区。(后续可能运用到多渲染目标技术)
	auto a = CurrentBackBufferView();
	auto b = DepthStencilView();
	mCommandList->OMSetRenderTargets(1, &a, true, &b);

	//再次对资源状态进行转换，将资源从渲染目标状态转换回呈现状态
	auto n = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	mCommandList->ResourceBarrier(1, &n);

	// 完成命令的记录
	ThrowIfFailed(mCommandList->Close());

	// 将待执行的命令列表加入命令队列
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// 交换后台缓冲区和前台缓冲区
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// 等待此帧的命令执行完毕。当前的实现没有什么效率，也过于简单
	//我们在后面将重新组织渲染部分的代码，以免在每一帧都要等待
	FlushCommandQueue();
}
