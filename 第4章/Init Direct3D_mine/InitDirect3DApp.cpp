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
	// Ϊ���԰汾��������ʱ�ڴ���,����ල�ڴ�й¶�����
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
	// �ظ�ʹ�ü�¼���������ڴ�
	//ֻ�е���GPU �����������б�ִ�����ʱ�����ǲ��ܽ�������
	ThrowIfFailed(mDirectCmdListAlloc->Reset());

	// ��ͨ��ExecuteCommandList������ĳ�������б����������к�
	//���Ǳ�������ø������б��Դ������������б����ڴ�
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	// ����Դ��״̬����ת��������Դ�ӳ���״̬ת��Ϊ��ȾĿ��״̬
	auto m = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	mCommandList->ResourceBarrier(1, &m);

	// �����ӿںͲü����Ρ�������Ҫ���������б�����ö�����
	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// �����̨����������Ȼ�����
	//ClearRenderTargetview�����Ὣָ������ȾĿ������Ϊ��������ɫ��ClearDepthStencilview��������������ָ�������/ģ�建��������ÿ֡Ϊ��ˢ�³�������ʼ����֮ǰ����������Ҫ�����̨��������ȾĿ������/ģ�建������
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// ָ����Ҫ��Ⱦ�Ļ�����
    //ID3D12GraphicsCommandList: : OMSetRenderTargets��ͨ���˷���������������ϣ������Ⱦ��ˮ����ʹ�õ���ȾĿ������/ģ�建������(�����������õ�����ȾĿ�꼼��)
	auto a = CurrentBackBufferView();
	auto b = DepthStencilView();
	mCommandList->OMSetRenderTargets(1, &a, true, &b);

	//�ٴζ���Դ״̬����ת��������Դ����ȾĿ��״̬ת���س���״̬
	auto n = CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	mCommandList->ResourceBarrier(1, &n);

	// �������ļ�¼
	ThrowIfFailed(mCommandList->Close());

	// ����ִ�е������б�����������
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// ������̨��������ǰ̨������
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// �ȴ���֡������ִ����ϡ���ǰ��ʵ��û��ʲôЧ�ʣ�Ҳ���ڼ�
	//�����ں��潫������֯��Ⱦ���ֵĴ��룬������ÿһ֡��Ҫ�ȴ�
	FlushCommandQueue();
}
