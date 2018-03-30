/***********************************************************
* Hook:http://blog.chinaunix.net/uid-660282-id-2414901.html
* Call:http://blog.csdn.net/yhz/article/details/1484073
* ���Ĵ���:https://github.com/YinTianliang/CAPIx
************************************************************/

#include <windows.h>
#include <gdiplus.h>
#include <wchar.h>
#include <iostream>
#include <cstdio>
#include <map>
using namespace std;
using namespace Gdiplus;

#define DLL_EXPORT __declspec(dllexport)
#define wcsicmp _wcsicmp
#define stricmp _stricmp
#define match(x,y) if (!stricmp(argv[x],y))
#define matchclsid(x) if (!stricmp(&argv[1][strlen(argv[1]) - 3], x))
#pragma comment(lib,"msimg32.lib")
#pragma comment(lib,"GdiPlus.lib")

struct imageres { //��Դ�ṹ��
	HDC dc;
	HBITMAP oldbmp;
	int w, h;
	imageres() {};
	imageres(char *file) //��ʼ���ṹ�壬��������Դ
	{
		wchar_t wfile[100];
		size_t converted = 0;
		mbstowcs_s(&converted, wfile, file, strlen(file));

		BITMAP bi;
		//Ϊ�˷�ֹ���hbmpͬʱ��һ��hdc������ͻ��������������е�hbmp������Ե�hdc
		dc = CreateCompatibleDC(nullptr);
		//HBITMAP bmp = (HBITMAP)LoadImageA(nullptr, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		HBITMAP bmp;
		Bitmap *bm = new Bitmap(wfile);
		bm->GetHBITMAP(0, &bmp);
		delete bm;
		oldbmp = (HBITMAP)SelectObject(dc, bmp);
		GetObject(bmp, sizeof(BITMAP), &bi);
		w = bi.bmWidth;
		h = bi.bmHeight;
	}
}*hTarget;
map<string, imageres> resmap; //��Դӳ���
HWND hCMD;//����̨���ھ��
char argv[10][100], info[50];
int drawdelay = 0;
double scale;//���ű�У��

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
	resmap["cmd"] = hRes; //��cmd��Ϊ��Դ��ӵ����ñ���
	hTarget = &resmap["cmd"];//getres("cmd"); //��ͼĬ��ָ��cmd
	//��ȡdesktop��С�Լ���ͼ���
	hDC = GetDC(nullptr);
	hRes.dc = hDC;
	hRes.w = dm.dmPelsWidth;
	hRes.h = dm.dmPelsHeight;
	resmap["desktop"] = hRes; //��desktop��Ϊ��Դ��ӵ����ñ���
	return;
}
imageres * getres(char *tag) //����Դӳ����в�����Դ
{
	if (!resmap.count(tag)) //�������Դӳ������Ҳ�����Դ�����ȼ���ͼƬ����Դӳ���
	{
		imageres hRes(tag);
		resmap[tag] = hRes;
	}
	return &resmap[tag];
}
void delres(char *tag) //����ԭ������Դ����ֹ�ڴ�й©
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
	double angle = -(double)atoi(argv[2]) / 180 * pi;
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
	double alpha = (double)atoi(argv[5])/100;
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
	int cx = atoi(argv[2]), cy = atoi(argv[3]);
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
	wchar_t **argvw = CommandLineToArgvW(CmdLine, &argc);
	//wchar_tתchar
	for (int i = 0; i < argc; i++)
	{
		size_t len = 2 * wcslen(argvw[i]) + 1;
		size_t converted = 0;
		wcstombs_s(&converted, argv[i], len, argvw[i], _TRUNCATE);
	}
	match(0, "help")
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
	match(0, "load") //������Դ����Դӳ���
	{
		char *tag; //��Դ������
		tag = (argc == 3) ? argv[2] : argv[1];
		//����ԭ������Դ����ֹ�ڴ�й©
		if (resmap.count(tag)) delres(tag);
		imageres hRes(argv[1]);
		resmap[tag] = hRes;
	}
	match(0, "unload") //ж����Դ
	{
		//����ԭ������Դ����ֹ�ڴ�й©
		delres(argv[1]);
	}
	match(0, "save") //����ΪͼƬ
	{
		imageres * hRes = getres(argv[2]);
		HBITMAP hSrc = copyhbitmap(hRes);
		Rect rect(0, 0, hRes->w, hRes->h);
		Bitmap bitmap(hSrc, nullptr);
		//https://stackoverflow.com/questions/1584202/gdi-bitmap-save-problem
		CLSID Clsid;
		matchclsid("bmp") CLSIDFromString(L"{557cf400-1a04-11d3-9a73-0000f81ef32e}", &Clsid);
		matchclsid("jpg") CLSIDFromString(L"{557cf401-1a04-11d3-9a73-0000f81ef32e}", &Clsid);
		matchclsid("png") CLSIDFromString(L"{557cf406-1a04-11d3-9a73-0000f81ef32e}", &Clsid);
		bitmap.Save(argvw[1], &Clsid, nullptr);
		DeleteObject(hSrc);
	}
	match(0, "target") //���Ļ�ͼĿ��
	{
		hTarget = getres(argv[1]);
	}
	match(0, "buffer") //�½�һ��buffer����
	{
		char *tag = argv[1];
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
	match(0, "resize") //����
	{
		match(1,"cmd")
		{
			RECT rc,rc2;
			SetScrollRange(hCMD, 0, 0, 0, 1);
			SetScrollRange(hCMD, 1, 0, 0, 1);
			GetClientRect(hCMD, &rc);
			GetWindowRect(hCMD, &rc2);
			int w = (rc2.right - rc2.left) - (rc.right - rc.left) + int((atoi(argv[2])) / scale);
			int h = (rc2.bottom - rc2.top) - (rc.bottom - rc.top) + int((atoi(argv[2])) / scale);
			//printf("scale:%f\n", scale);
			//printf("C:%dx%d\n", rc.right - rc.left, rc.bottom - rc.top);
			//printf("W:%dx%d\n", rc2.right - rc2.left, rc2.bottom - rc2.top);
			MoveWindow(hCMD, rc2.left, rc2.top, w, h, 0);
			Sleep(10);
			SetScrollRange(hCMD, 0, 0, 0, 1);
			SetScrollRange(hCMD, 1, 0, 0, 1);
		}else{
			imageres * hRes = getres(argv[1]);
			HDC hDCMem = CreateCompatibleDC(hRes->dc);
			HBITMAP hBitmap = CreateCompatibleBitmap(hRes->dc, atoi(argv[2]), atoi(argv[3]));
			HBITMAP oldbmp = (HBITMAP)SelectObject(hDCMem, hBitmap);
			StretchBlt(hDCMem, 0, 0, atoi(argv[2]), atoi(argv[3]), hRes->dc, 0, 0, hRes->w, hRes->h, SRCCOPY);
			//����ԭ������Դ����ֹ�ڴ�й©
			HBITMAP bmp = (HBITMAP)SelectObject(hRes->dc, hRes->oldbmp);
			DeleteObject(bmp);
			DeleteDC(hRes->dc);
			//�滻ԭ������Դ
			hRes->oldbmp = oldbmp;
			hRes->dc = hDCMem;
			hRes->w = atoi(argv[2]);
			hRes->h = atoi(argv[3]);
		}
	}
	match(0, "cls") //����
	{
		InvalidateRect(hCMD, nullptr, true);
	}
	match(0, "rotate")
	{
		rotateres();
	}
	match(0, "draw")
	{
		//ֱ����Ŀ���ϻ�ͼ
		imageres * hRes = getres(argv[1]);
		if (argc == 4)
		{
				BitBlt(hTarget->dc, atoi(argv[2]), atoi(argv[3]), hRes->w, hRes->h, hRes->dc, 0, 0, SRCCOPY);
		}
		else
		{
			match(4, "trans")
					TransparentBlt(hTarget->dc, atoi(argv[2]), atoi(argv[3]), hRes->w, hRes->h, hRes->dc, 0, 0, hRes->w, hRes->h, RGB(255, 255, 255));
			match(4, "alpha")
				alphares();
		}
		Sleep(drawdelay);
	}
	match(0, "delay")
	{
		drawdelay = atoi(argv[1]);
	}
	match(0, "info")
	{
		imageres * hRes = getres(argv[1]);
		sprintf_s(info, sizeof(info), "%d %d", hRes->w, hRes->h);
		SetEnvironmentVariableA("image", info);
	}
	match(0, "export")
	{
		sprintf_s(info, sizeof(info), "%d", (int)hCMD);
		SetEnvironmentVariableA("image", info);
	}
	match(0, "import")
	{
		char *tag = argv[2];
		//����ԭ������Դ����ֹ�ڴ�й©
		if (resmap.count(tag)) delres(tag);
		imageres hRes;
		//��ȡcmd��С�Լ���ͼ���
		HWND hCMD2 = (HWND)atoi(argv[1]);
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
	match(0, "getpix")
	{
		imageres * hRes = getres(argv[1]);
		COLORREF color=GetPixel(hRes->dc, atoi(argv[2]), atoi(argv[3]));
		sprintf_s(info, sizeof(info), "%d %d %d", GetRValue(color), GetGValue(color), GetBValue(color));
		SetEnvironmentVariableA("image", info);
	}
	match(0, "setpix")
	{
		imageres * hRes = getres(argv[1]);
		SetPixel(hRes->dc, atoi(argv[2]), atoi(argv[3]), RGB(atoi(argv[4]), atoi(argv[5]), atoi(argv[6])));
	}
	LocalFree(argvw);
	return;
}
