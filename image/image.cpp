/***********************************************************
* Hook:http://blog.chinaunix.net/uid-660282-id-2414901.html
* Call:http://blog.csdn.net/yhz/article/details/1484073
* ���Ĵ���:https://github.com/YinTianliang/CAPIx
************************************************************/

#include <windows.h>
#include <gdiplus.h>
#include <wchar.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <map>
using namespace std;
using namespace Gdiplus;

#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0) 
#define DLL_EXPORT __declspec(dllexport)
#define wtoi _wtoi
#define wcsicmp _wcsicmp
#define match(x,y) if (!wcsicmp(argv[x],y))
#define matchclsid(x) if (!wcsicmp(&argv[1][wcslen(argv[1]) - 3], x))
#pragma comment(lib,"msimg32.lib")
#pragma comment(lib,"GdiPlus.lib")

struct imageres { //��Դ�ṹ��
	HDC dc;
	HBITMAP oldbmp;
	int w, h;
	imageres() {};
	imageres(wchar_t *file) //��ʼ���ṹ�壬��������Դ
	{
		BITMAP bi;
		//Ϊ�˷�ֹ���hbmpͬʱ��һ��hdc������ͻ��������������е�hbmp������Ե�hdc
		dc = CreateCompatibleDC(nullptr);
		//HBITMAP bmp = (HBITMAP)LoadImageA(nullptr, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		HBITMAP bmp;
		Bitmap *bm = new Bitmap(file);
		bm->GetHBITMAP(0, &bmp);
		delete bm;
		oldbmp = (HBITMAP)SelectObject(dc, bmp);
		GetObject(bmp, sizeof(BITMAP), &bi);
		w = bi.bmWidth;
		h = bi.bmHeight;
	}
}*hTarget;
map<wstring, imageres> resmap; //��Դӳ���
HWND hCMD;//����̨���ھ��
double scale;//У�����ű�
wchar_t **argv;

void image(wchar_t *); //������
void Init_image(); //��ʼ��

bool WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpvReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		//HookAPI(SetEnvironmentVariableW, SetCall_image);
		DisableThreadLibraryCalls(hModule);
		Init_image();
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return true;
}
extern "C" DLL_EXPORT int WINAPI Init(void)//��������������,�������
{
	return 0;
}
extern "C" __declspec(dllexport) void call(wchar_t *varName, wchar_t *varValue)
{
	//�жϱ������Ƿ�Ϊimage, �������image
	if (!wcsicmp(varName, L"image")) image(varValue);
	return;
}

void Init_image() //��ʼ��
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

	imageres hRes;
	//��ȡcmd��С�Լ���ͼ���
	hCMD = GetConsoleWindow();
	HDC hDC = GetDC(hCMD);
	DEVMODE dm;
	dm.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dm);
	int ax = dm.dmPelsWidth;
	int bx = GetSystemMetrics(SM_CXSCREEN);
	scale = (double)ax / bx;//У�����ű�
	RECT rc;
	GetClientRect(hCMD, &rc);
	hRes.dc = hDC;
	hRes.w = int(scale*(rc.right - rc.left));
	hRes.h = int(scale*(rc.bottom - rc.top));
	resmap[L"cmd"] = hRes; //��cmd��Ϊ��Դ��ӵ����ñ���
	hTarget = &resmap[L"cmd"];//getres("cmd"); //��ͼĬ��ָ��cmd
	//��ȡdesktop��С�Լ���ͼ���
	hDC = GetDC(nullptr);
	hRes.dc = hDC;
	hRes.w = dm.dmPelsWidth;
	hRes.h = dm.dmPelsHeight;
	resmap[L"desktop"] = hRes; //��desktop��Ϊ��Դ��ӵ����ñ���

	TextOutA(hTarget->dc, 0, 0, 0, 0);//��һ��ʹ��TextOutA��Ч������Ǹ�bug
	return;
}
imageres * getres(wchar_t *tag) //����Դӳ����в�����Դ
{
	if (!resmap.count(tag)) //�������Դӳ������Ҳ�����Դ�����ȼ���ͼƬ����Դӳ���
	{
		imageres hRes(tag);
		resmap[tag] = hRes;
	}
	return &resmap[tag];
}
void delres(wchar_t *tag) //����ԭ������Դ����ֹ�ڴ�й©
{
	imageres * hRes = getres(tag);
	HBITMAP bmp = (HBITMAP)SelectObject(hRes->dc, hRes->oldbmp);
	DeleteObject(bmp);
	DeleteDC(hRes->dc);
	resmap.erase(tag);
	return;
}
//������SelectObject��ȡcmd��������Դ��hbitmap������Ҫ����һ�ݳ�����ע��ʹ��֮��ҪDeleteObject
HBITMAP copyhbitmap(imageres *hSrc)
{
	imageres hRes;
	hRes.dc = CreateCompatibleDC(hSrc->dc);
	HBITMAP hBitmap = CreateCompatibleBitmap(hSrc->dc, hSrc->w, hSrc->h);
	hRes.oldbmp = (HBITMAP)SelectObject(hRes.dc, hBitmap);
	BitBlt(hRes.dc, 0, 0, hSrc->w, hSrc->h, hSrc->dc, 0, 0, SRCCOPY);
	SelectObject(hRes.dc, hRes.oldbmp);
	DeleteDC(hRes.dc);
	return hBitmap;
}
void rotateres()
{
	imageres * hRes = getres(argv[1]);
	HBITMAP hSrc = copyhbitmap(hRes);
	Rect rect(0, 0, hRes->w, hRes->h);
	//���ڼ��ؾ�λͼ
	Bitmap bitmap(hSrc, nullptr);
	BitmapData bitmapData;
	bitmap.LockBits(&rect, ImageLockModeRead, PixelFormat24bppRGB, &bitmapData);
	byte* pixels = (byte*)bitmapData.Scan0;
	//���ڼ�����λͼ
	Bitmap bitmap2(hSrc, nullptr);
	BitmapData bitmapData2;
	bitmap2.LockBits(&rect, ImageLockModeWrite, PixelFormat24bppRGB, &bitmapData2);
	byte* pixels2 = (byte*)bitmapData2.Scan0;
	//��ת
	double pi = 3.1415926;
	double angle = -(double)wtoi(argv[2]) / 180 * pi;
	double sina = sin(angle), cosa = cos(angle);
	int cx = hRes->w / 2, cy = hRes->h / 2;
	for (int i = 0; i<hRes->w; i++)
		for (int j = 0; j<hRes->h; j++)
		{
			int x = (int)(cx + (i - cx)*cosa - (j - cy)*sina), y = (int)(cy + (i - cx)*sina + (j - cy)*cosa);//ԭ����
			if (x >= 0 && x < hRes->w&&y >= 0 && y < hRes->h)
			{
				for (int k = 0; k < 3; k++)
					pixels2[j*bitmapData2.Stride + 3 * i + k] = pixels[y*bitmapData.Stride + 3 * x + k];
			}
			else
			{
				for (int k = 0; k < 3; k++)
					pixels2[j*bitmapData2.Stride + 3 * i + k] = 0xFF;
			}
		}
	bitmap.UnlockBits(&bitmapData);
	bitmap2.UnlockBits(&bitmapData2);
	//������ʱ��Դ��Ŀ����Դ
	HDC hDCMem = CreateCompatibleDC(hRes->dc);
	HBITMAP hBitmap;
	bitmap2.GetHBITMAP(0, &hBitmap);
	HBITMAP oldbmp = (HBITMAP)SelectObject(hDCMem, hBitmap);
	BitBlt(hRes->dc, 0, 0, hRes->w, hRes->h, hDCMem, 0, 0, SRCCOPY);
	//������ʱ���Ƶ���Դ
	DeleteObject(hSrc);
	SelectObject(hDCMem, oldbmp);
	DeleteObject(hBitmap);
	DeleteDC(hDCMem);
}
void alphares()
{
	double alpha = (double)wtoi(argv[5])/100;
	//���ڼ���Դλͼ
	imageres * hRes = getres(argv[1]);
	HBITMAP hSrc = copyhbitmap(hRes);
	Rect rect(0, 0, hRes->w, hRes->h);
	Bitmap bitmap(hSrc, nullptr);
	BitmapData bitmapData;
	bitmap.LockBits(&rect, ImageLockModeRead, PixelFormat24bppRGB, &bitmapData);
	byte* pixels = (byte*)bitmapData.Scan0;
	//���ڼ���Ŀ��λͼ
	//����SelectObject��ȡcmd��������Դ��hbitmap������Ҫ����һ�ݳ�����ע��ʹ��֮��ҪDeleteObject
	HBITMAP hSrc2 = copyhbitmap(hTarget);
	Rect rect2(0, 0, hTarget->w, hTarget->h);
	Bitmap bitmap2(hSrc2, nullptr);
	BitmapData bitmapData2;
	bitmap2.LockBits(&rect2, ImageLockModeRead, PixelFormat24bppRGB, &bitmapData2);
	byte* pixels2 = (byte*)bitmapData2.Scan0;
	//���ڼ�����λͼ
	Rect rect3(0, 0, hTarget->w, hTarget->h);
	Bitmap bitmap3(hSrc2, nullptr);
	BitmapData bitmapData3;
	bitmap3.LockBits(&rect3, ImageLockModeWrite, PixelFormat24bppRGB, &bitmapData3);
	byte* pixels3 = (byte*)bitmapData3.Scan0;
	//alpha���
	int cx = wtoi(argv[2]), cy = wtoi(argv[3]);
	for (int i = 0; i<hTarget->w; i++)
		for (int j = 0; j<hTarget->h; j++)
		{
			int x = i - cx, y = j - cy;//Դ����
			if (x >= 0 && x < hRes->w&&y >= 0 && y < hRes->h)
			{
				for (int k = 0; k < 3; k++)
					pixels3[j*bitmapData3.Stride + 3 * i + k] =
					(byte)((1 - alpha) * pixels2[j*bitmapData2.Stride + 3 * i + k] +
						alpha * pixels[y*bitmapData.Stride + 3 * x + k]);
			}
			else
			{
				for (int k = 0; k < 3; k++)
					pixels3[j*bitmapData3.Stride + 3 * i + k] = pixels2[j*bitmapData2.Stride + 3 * i + k];
			}
		}
	bitmap.UnlockBits(&bitmapData);
	bitmap2.UnlockBits(&bitmapData2);
	bitmap3.UnlockBits(&bitmapData3);
	//������ʱ��Դ��Ŀ����Դ
	HDC hDCMem = CreateCompatibleDC(hTarget->dc);
	HBITMAP hBitmap;
	bitmap3.GetHBITMAP(0, &hBitmap);
	HBITMAP oldbmp = (HBITMAP)SelectObject(hDCMem, hBitmap);
	BitBlt(hTarget->dc, 0, 0, hTarget->w, hTarget->h, hDCMem, 0, 0, SRCCOPY);
	//������ʱ���Ƶ���Դ
	DeleteObject(hSrc);
	DeleteObject(hSrc2);
	SelectObject(hDCMem, oldbmp);
	DeleteObject(hBitmap);
	DeleteDC(hDCMem);
}

void image(wchar_t *CmdLine)
{
	int argc;
	argv = CommandLineToArgvW(CmdLine, &argc);
	match(0, L"help")
	{
		printf(
			"image\n"
			"����̨��ʾͼƬ Ver 3.0 by Byaidu\n"
			"\n"
			"help\t\t\t��ʾ����\n"
			"load file [tag]\t\t����һ�黭��tag��������ͼƬ������tag\n"
			"unload tag\t\tɾ������tag\n"
			"save file tag\t\t������tag�����ݴ洢��file��\n"
			"target tag\t\t�л���ǰ��ͼĿ��Ϊ����tag\n"
			"buffer tag\t\t����һ�黭��tag\n"
			"stretch tag w h\t\t������tag���ŵ�w*h�Ĵ�С\n"
			"cls\t\t\t��ջ���cmd������\n"
			"rotate tag degree\t������tag˳ʱ����תdegree��\n"
			"draw tag x y [trans|and]������tag���Ƶ���ǰ��ͼĿ���x,yλ����\n"
			"info tag\t\t������tag�Ŀ�͸ߴ洢������image\n"
			"export\t\t\t������cmd�ľ���洢������image\n"
			"import handle tag\tͨ���������һ������̨�Ļ���cmdӳ�䵽�˿���̨�Ļ���tag\n"
			"getpix tag x y\t\t������tag��x,yλ�õ�rgbֵ�洢������image\n"
			"setpix tag x y r g b\t���û���tag��x,yλ�õ�rgbֵ\n"
		);
	}
	match(0, L"load") //������Դ����Դӳ���
	{
		wchar_t *tag; //��Դ������
		tag = (argc == 3) ? argv[2] : argv[1];
		//����ԭ������Դ����ֹ�ڴ�й©
		if (resmap.count(tag)) delres(tag);
		imageres hRes(argv[1]);
		resmap[tag] = hRes;
	}
	match(0, L"unload") //ж����Դ
	{
		//����ԭ������Դ����ֹ�ڴ�й©
		delres(argv[1]);
	}
	match(0, L"save") //����ΪͼƬ
	{
		imageres * hRes = getres(argv[2]);
		HBITMAP hSrc = copyhbitmap(hRes);
		Rect rect(0, 0, hRes->w, hRes->h);
		Bitmap bitmap(hSrc, nullptr);
		//https://stackoverflow.com/questions/1584202/gdi-bitmap-save-problem
		CLSID Clsid;
		matchclsid(L"bmp") CLSIDFromString(L"{557cf400-1a04-11d3-9a73-0000f81ef32e}", &Clsid);
		matchclsid(L"jpg") CLSIDFromString(L"{557cf401-1a04-11d3-9a73-0000f81ef32e}", &Clsid);
		matchclsid(L"png") CLSIDFromString(L"{557cf406-1a04-11d3-9a73-0000f81ef32e}", &Clsid);
		bitmap.Save(argv[1], &Clsid, nullptr);
		DeleteObject(hSrc);
	}
	match(0, L"target") //���Ļ�ͼĿ��
	{
		hTarget = getres(argv[1]);
	}
	match(0, L"buffer") //�½�һ��buffer����
	{
		wchar_t *tag = argv[1];
		//����ԭ������Դ����ֹ�ڴ�й©
		if (resmap.count(tag)) delres(tag);
		imageres hRes;
		hRes.dc = CreateCompatibleDC(hTarget->dc);
		HBITMAP hBitmap = CreateCompatibleBitmap(hTarget->dc, hTarget->w, hTarget->h);
		hRes.oldbmp = (HBITMAP)SelectObject(hRes.dc, hBitmap);
		BitBlt(hRes.dc, 0, 0, hTarget->w, hTarget->h, nullptr, 0, 0, WHITENESS);
		hRes.w = hTarget->w;
		hRes.h = hTarget->h;
		//��buffer��ӵ���Դ���ñ���
		resmap[tag] = hRes;
	}
	match(0, L"resize") //����
	{
		imageres * hRes = getres(argv[1]);
		match(1,L"cmd")
		{
			RECT rc,rc2;
			SetScrollRange(hCMD, 0, 0, 0, 1);
			SetScrollRange(hCMD, 1, 0, 0, 1);
			GetClientRect(hCMD, &rc);
			GetWindowRect(hCMD, &rc2);
			int w = (rc2.right - rc2.left) - (rc.right - rc.left) + int((wtoi(argv[2])) / scale);
			int h = (rc2.bottom - rc2.top) - (rc.bottom - rc.top) + int((wtoi(argv[3])) / scale);
			//printf("scale:%f\n", scale);
			//printf("C:%dx%d\n", rc.right - rc.left, rc.bottom - rc.top);
			//printf("W:%dx%d\n", rc2.right - rc2.left, rc2.bottom - rc2.top);
			MoveWindow(hCMD, rc2.left, rc2.top, w, h, 0);
			Sleep(10);
			SetScrollRange(hCMD, 0, 0, 0, 1);
			SetScrollRange(hCMD, 1, 0, 0, 1);
			Sleep(10);
			hRes->w = (int)wtoi(argv[2]);
			hRes->h = (int)wtoi(argv[3]);
		}else{
			HDC hDCMem = CreateCompatibleDC(hRes->dc);
			HBITMAP hBitmap = CreateCompatibleBitmap(hRes->dc, wtoi(argv[2]), wtoi(argv[3]));
			HBITMAP oldbmp = (HBITMAP)SelectObject(hDCMem, hBitmap);
			StretchBlt(hDCMem, 0, 0, wtoi(argv[2]), wtoi(argv[3]), hRes->dc, 0, 0, hRes->w, hRes->h, SRCCOPY);
			//����ԭ������Դ����ֹ�ڴ�й©
			HBITMAP bmp = (HBITMAP)SelectObject(hRes->dc, hRes->oldbmp);
			DeleteObject(bmp);
			DeleteDC(hRes->dc);
			//�滻ԭ������Դ
			hRes->oldbmp = oldbmp;
			hRes->dc = hDCMem;
			hRes->w = wtoi(argv[2]);
			hRes->h = wtoi(argv[3]);
		}
	}
	match(0, L"cls") //����
	{
		InvalidateRect(hCMD, nullptr, true);
	}
	match(0, L"rotate")
	{
		rotateres();
	}
	match(0, L"draw")
	{
		//ֱ����Ŀ���ϻ�ͼ
		imageres * hRes = getres(argv[1]);
		if (argc == 4)
		{
				BitBlt(hTarget->dc, wtoi(argv[2]), wtoi(argv[3]), hRes->w, hRes->h, hRes->dc, 0, 0, SRCCOPY);
		}
		else
		{
			match(4, L"trans")
					TransparentBlt(hTarget->dc, wtoi(argv[2]), wtoi(argv[3]), hRes->w, hRes->h, hRes->dc, 0, 0, hRes->w, hRes->h, RGB(255, 255, 255));
			match(4, L"alpha")
				alphares();
		}
	}
	match(0, L"text")
	{
		//��ʾ���βŻ�ˢ�³���������Ǹ�bug
		for (int i = 0; i < 2;i++) TextOutW(hTarget->dc, wtoi(argv[2]), wtoi(argv[3]), argv[1], wcslen(argv[1]));
	}
	match(0, L"font")
	{
		SetBkMode(hTarget->dc, TRANSPARENT);
		SetTextColor(hTarget->dc, RGB(wtoi(argv[3]), wtoi(argv[4]), wtoi(argv[5])));
		HFONT hFont = CreateFontW(
			wtoi(argv[2]), wtoi(argv[1]), 0/*���ù�*/, 0/*���ù�*/, 400 /*һ�����ֵ��Ϊ400*/,
			FALSE/*����б��*/, FALSE/*�����»���*/, FALSE/*����ɾ����*/,
			DEFAULT_CHARSET, //��������ʹ��Ĭ���ַ��������������� _CHARSET ��β�ĳ�������
			OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, //���в������ù�
			DEFAULT_QUALITY, //Ĭ���������
			FF_DONTCARE, //��ָ��������*/
			L"������" //������
		);
		SelectObject(hTarget->dc,hFont);
	}
	match(0, L"sleep")
	{
		Sleep(wtoi(argv[1]));
	}
	match(0, L"info")
	{
		wchar_t info[100];
		imageres * hRes = getres(argv[1]);
		wsprintfW(info, L"%d %d", hRes->w, hRes->h);
		SetEnvironmentVariableW(L"image", info);
	}
	match(0, L"export")
	{
		wchar_t info[100];
		wsprintfW(info, L"%d", (int)hCMD);
		SetEnvironmentVariableW(L"image", info);
	}
	match(0, L"import")
	{
		wchar_t *tag = argv[2];
		//����ԭ������Դ����ֹ�ڴ�й©
		if (resmap.count(tag)) delres(tag);
		imageres hRes;
		//��ȡcmd��С�Լ���ͼ���
		HWND hCMD2 = (HWND)wtoi(argv[1]);
		HDC hDC = GetDC(hCMD2);
		DEVMODE dm;
		dm.dmSize = sizeof(DEVMODE);
		EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dm);
		int ax = dm.dmPelsWidth;
		int bx = GetSystemMetrics(SM_CXSCREEN);
		double scale = (double)ax / bx;//У�����ű�
		RECT rc;
		GetClientRect(hCMD2, &rc);
		hRes.dc = hDC;
		hRes.w = (int)ceil(scale*(rc.right - rc.left));
		hRes.h = (int)ceil(scale*(rc.bottom - rc.top));
		resmap[tag] = hRes; //��cmd��Ϊ��Դ��ӵ����ñ���
	}
	match(0, L"getpix")
	{
		wchar_t info[100];
		COLORREF color=GetPixel(hTarget->dc, wtoi(argv[1]), wtoi(argv[2]));
		wsprintfW(info, L"%d %d %d", GetRValue(color), GetGValue(color), GetBValue(color));
		SetEnvironmentVariableW(L"image", info);
	}
	match(0, L"setpix")
	{
		SetPixel(hTarget->dc, wtoi(argv[1]), wtoi(argv[2]), RGB(wtoi(argv[3]), wtoi(argv[4]), wtoi(argv[5])));
	}
	match(0, L"list")
	{
		ifstream in(argv[1]);
		string str;
		wchar_t wstr[100];
		while (!in.eof())
		{
			getline(in, str);
			MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wstr, sizeof(wstr));
			image(wstr);
		}
		in.close();
	}
	match(0, L"mouse")
	{
		imageres *hRes = getres((wchar_t*)L"cmd");
		wchar_t info[100];
		POINT mosPos;
		int x, y;
		// ��ȡ��׼��������豸���  
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
		DWORD oldConMode;
		GetConsoleMode(hIn, &oldConMode); // ����
		SetConsoleMode(hIn, (oldConMode | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT)&(~ENABLE_QUICK_EDIT_MODE) );
		INPUT_RECORD	mouseRec;
		DWORD			res;
		while (1)
		{
			ReadConsoleInput(hIn, &mouseRec, 1, &res);
			if (mouseRec.EventType == MOUSE_EVENT)
			{
				if (mouseRec.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
				{
					GetCursorPos(&mosPos);
					ScreenToClient(hCMD, &mosPos);
					x = min(max((int)scale*mosPos.x, 0), hRes->w);
					y = min(max((int)scale*mosPos.y, 0), hRes->h);
					break;
				}
			}
		}
		wsprintfW(info, L"%d %d", x, y);
		SetEnvironmentVariableW(L"image", info);
		SetConsoleMode(hIn, oldConMode);
	}
	LocalFree(argv);
	return;
}
