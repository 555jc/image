// image.cpp : Defines the entry point for the console application.
//
//�汾��¼:
//Ver 1.0 ԭ��
//Ver 1.1 ������/?����ѡ��
//Ver 1.2 ������/d���ͼƬѡ��
//Ver 1.3 �����ø��ֹ�դ�����룬����/sѡ�Ч�ʸ��� 
//Ver 1.4 �޸�/dѡ���BUG
//Ver 1.5 �޸��ڴ�й¶
//Ver 1.6 ������/TRANSPARENTBLTѡ��
//Ver 1.7 �����ִ�Сд��������/l��������ѡ��޸�/dѡ���BUG
//Ver 1.8 /l֧��˫����

#include <fstream>
#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

void Draw(HDC hDC,HDC hDCMem,int imx,int imy,BITMAP bi,const char *dwRop);

int main(int argc, char *argv[])
{
	if (argc == 2 && _stricmp(argv[1], "/?") == 0 || argc == 1)
	{
		printf("����̨��ʾͼƬ\n"
			"Ver 1.8 By Byaidu\n\n"
			"image [/d] [/s] [/l listfile] [bmpfile] [X] [Y] [Width] [Height] [/TRANSPARENTBLT] [/BLACKNESS] [/DSTINVERT] [/MERGECOPY] [/MERGEPAINT] [/NOTSRCCOPY] [/NOTSRCERASE] [/PATCOPY] [/PATPAINT] [/PATINVERT] [/SRCAND] [/SRCCOPY] [/SRCERASE] [/SRCINVERT] [/SRCPAINT] [/WHITENESS] \n\n"
			"\t/d\t��յ�ǰ��������ʾ��ͼƬ\n"
			"\t/s\t��յ�ǰ����ָ����С����ʾ��ͼƬ\n"
			"\t/l listfile\tʹ��ָ���ļ���������\n"
			"\tbmpfile\tָ��Ҫ��ʾ��ͼƬ\n"
			"\tX\tָ��Ŀ������������Ͻǵ�X���߼�����\n"
			"\tY\tָ��Ŀ������������Ͻǵ�Y���߼�����\n"
			"\tWidth\tΪ/sѡ��ָ��Ŀ�����������߼���ȡ�\n"
			"\tHeight\tΪ/sѡ��ָ��Ŀ�����������߼��߶ȡ�\n"
			"\t/TRANSPARENTBLT\t��ʾ��Դλͼ�е�RGBֵΪ255,255,255����͸����ɫ������Ŀ���������\n"
			"\t/BLACKNESS\t��ʾʹ���������ɫ�������0��ص�ɫ�������Ŀ��������򣬣���ȱʡ�������ɫ����ԣ�����ɫΪ��ɫ��\n"
			"\t/DSTINVERT\t��ʾʹĿ�����������ɫȡ��\n"
			"\t/MERGECOPY\t��ʾʹ�ò����͵�AND���룩��������Դ�����������ɫ���ض�ģʽ���һ��\n"
			"\t/MERGEPAINT\tͨ��ʹ�ò����͵�OR���򣩲������������Դ�����������ɫ��Ŀ������������ɫ�ϲ�\n"
			"\t/NOTSRCCOPY\t��Դ����������ɫȡ�����ڿ�����Ŀ���������\n"
			"\t/NOTSRCERASE\tʹ�ò������͵�OR���򣩲��������Դ��Ŀ������������ɫֵ��Ȼ�󽫺ϳɵ���ɫȡ��\n"
			"\t/PATCOPY\t���ض���ģʽ������Ŀ��λͼ��\n"
			"\t/PATPAINT\tͨ��ʹ�ò���OR���򣩲�������Դ��������ȡ�������ɫֵ���ض�ģʽ����ɫ�ϲ�Ȼ��ʹ��OR���򣩲��������ò����Ľ����Ŀ����������ڵ���ɫ�ϲ�\n"
			"\t/PATINVERT\tͨ��ʹ��XOR����򣩲�������Դ��Ŀ����������ڵ���ɫ�ϲ�\n"
			"\t/SRCAND\tͨ��ʹ��AND���룩����������Դ��Ŀ����������ڵ���ɫ�ϲ�\n"
			"\t/SRCCOPY\t��Դ��������ֱ�ӿ�����Ŀ���������\n"
			"\t/SRCERASE\tͨ��ʹ��AND���룩��������Ŀ�����������ɫȡ������Դ�����������ɫֵ�ϲ�\n"
			"\t/SRCINVERT\tͨ��ʹ�ò����͵�XOR����򣩲�������Դ��Ŀ������������ɫ�ϲ�"
			"\t/SRCPAINT\tͨ��ʹ�ò����͵�OR���򣩲�������Դ��Ŀ������������ɫ�ϲ�\n"
			"\t/WHITENESS\tʹ���������ɫ��������1�йص���ɫ���Ŀ��������򣨶���ȱʡ�����ɫ����˵�������ɫ���ǰ�ɫ��\n\n"
			"��ֱ��ʹ��image abc.bmp 10 50 ���Ӻ���Ĳ�����Ĭ��ʹ��/SRCCOPY\n"
			"ע�⣬X,Y�����ɵ�������0����\n"
			"���磺\n"
			"\timage abc.bmp 10 50\n"
			"\timage abc.bmp 10 50 /SRCAND\n"
			"\timage /d\n"
			"\timage /s 10 10 100 100\n");
		return 0;
	}
	HWND hCMD = GetConsoleWindow();
	HDC hDC = GetDC(hCMD);
	HDC hDCMem = CreateCompatibleDC(hDC);//����ͼ��
	if (argc == 3 && _stricmp(argv[1], "/l") == 0)
	{
		RECT rc; 
		int width; 
		int height; 
		GetClientRect(hCMD, &rc); /* hwnd Ϊ���ھ�� */ 
		width = rc.right - rc.left; /* �� */ 
		height = rc.bottom - rc.top; /* �� */ 
		HDC hDCBuffer = CreateCompatibleDC(hDC);//˫����
		HBITMAP hBitmapBuffer = CreateCompatibleBitmap(hDC,width,height);
		SelectObject(hDCBuffer, hBitmapBuffer);
		HBITMAP hBitmapX = CreateCompatibleBitmap(hDC,width,height);
		SelectObject(hDC, hBitmapX);
		ifstream in;
		string str;
		string sTmp[3];
		in.open(argv[2]);
		while(!in.eof())
		{
			int i=0;
			getline(in,str);
			istringstream istr(str);
			while(!istr.eof())
			{
				istr>>sTmp[i];
				i++;
			}
			HBITMAP hBitmap;
			hBitmap = (HBITMAP)LoadImageA(NULL, (char*)sTmp[0].data(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
			SelectObject(hDCMem, hBitmap);
			BITMAP bi = {0};
			GetObject(hBitmap, sizeof(BITMAP), &bi);
			if (i == 3) BitBlt(hDCBuffer,atoi(sTmp[1].c_str()),atoi(sTmp[2].c_str()),bi.bmWidth,bi.bmHeight,hDCMem,0,0,SRCCOPY);
			if (i == 4) Draw(hDCBuffer,hDCMem,atoi(sTmp[1].c_str()),atoi(sTmp[2].c_str()),bi,(char*)sTmp[3].data());
		}
		in.close();
		BitBlt(hDC, 0, 0, width, height, hDCBuffer, 0, 0, SRCCOPY);
		return 0;
	}
	if (argc == 2 && _stricmp(argv[1], "/d") == 0)
	{
		InvalidateRect(hCMD, NULL, true);
		return 0;
	}
	int imx = atoi(argv[2]);
	int imy = atoi(argv[3]);
	if (argc == 6 && _stricmp(argv[1], "/s") == 0)
	{
		int imw = atoi(argv[4]);
		int imh = atoi(argv[5]);
		BitBlt(hDC, imx, imy, imw, imh, NULL, 0, 0, BLACKNESS);
		return 0;
	}
	HBITMAP hBitmap;
	hBitmap = (HBITMAP)LoadImageA(NULL, argv[1], IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	SelectObject(hDCMem, hBitmap);
	BITMAP bi = {0};
	GetObject(hBitmap, sizeof(BITMAP), &bi);
	if (argc == 4) Draw(hDC,hDCMem,imx,imy,bi,"SRCCOPY");
	if (argc == 5) Draw(hDC,hDCMem,imx,imy,bi,argv[4]);
	DeleteObject(hBitmap);
	DeleteDC(hDCMem);
	ReleaseDC(hCMD, hDC);
	return 0;
}

void Draw(HDC hDC,HDC hDCMem,int imx,int imy,BITMAP bi,const char *dwRop)
{
	if (_stricmp(dwRop, "/TRANSPARENTBLT") == 0)
	{
		GdiTransparentBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, bi.bmWidth, bi.bmHeight, RGB(255,255,255));
	}
	if (_stricmp(dwRop, "/BLACKNESS") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, BLACKNESS);
	}
	if (_stricmp(dwRop, "/DSTINVERT") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, DSTINVERT);
	}
	if (_stricmp(dwRop, "/MERGECOPY") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, MERGECOPY);
	}
	if (_stricmp(dwRop, "/MERGEPAINT") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, MERGEPAINT);
	}
	if (_stricmp(dwRop, "/NOTSRCCOPY") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, NOTSRCCOPY);
	}
	if (_stricmp(dwRop, "/NOTSRCERASE") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, NOTSRCERASE);
	}
	if (_stricmp(dwRop, "/PATCOPY") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, PATCOPY);
	}
	if (_stricmp(dwRop, "/PATPAINT") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, PATPAINT);
	}
	if (_stricmp(dwRop, "/PATINVERT") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, PATINVERT);
	}
	if (_stricmp(dwRop, "/SRCAND") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, SRCAND);
	}
	if (_stricmp(dwRop, "/SRCCOPY") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, SRCCOPY);
	}
	if (_stricmp(dwRop, "/SRCERASE") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, SRCERASE);
	}
	if (_stricmp(dwRop, "/SRCINVERT") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, SRCINVERT);
	}
	if (_stricmp(dwRop, "/SRCPAINT") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, SRCPAINT);
	}
	if (_stricmp(dwRop, "/WHITENESS") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, WHITENESS);
	}
}
