// University.MemoryCard.cpp : Определяет точку входа для приложения.
//
#include <Windows.h>
#include <tlhelp32.h>
#include <commctrl.h>
#include "Resource.h"
#include "framework.h"

#pragma comment(lib, "comctl32.lib")
#include "University.MemoryCard.h"

#define MAX_LOADSTRING 100

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

HWND hListView, hListBox, hEditMin, hEditMax, hButtonFilter;
HWND hMemoryCountText;  // Поле с количеством регионов
DWORD selectedProcessID = 0;

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void AddListViewColumns(HWND hWndListView);
void AddListViewItem(HWND hWndListView, int index, MEMORY_BASIC_INFORMATION mbi);
void PrintMemoryInfo(HWND hWndListView, DWORD processID, LPVOID minAddress, LPVOID maxAddress);
const wchar_t* GetProtectString(DWORD protect);
const wchar_t* GetTypeString(DWORD type);
const wchar_t* GetStateString(DWORD state);

void PopulateProcessList(HWND hWndComboBox);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Разместите код здесь.

	// Инициализация глобальных строк
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_UNIVERSITYMEMORYCARD, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Выполнить инициализацию приложения:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_UNIVERSITYMEMORYCARD));

	MSG msg;

	// Цикл основного сообщения:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_UNIVERSITYMEMORYCARD));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_UNIVERSITYMEMORYCARD);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		InitCommonControls();
		hListBox = CreateWindow(WC_LISTBOX, L"", WS_VSCROLL | WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_BORDER,
			10, 10, 200, 200, hWnd, (HMENU)2, hInst, NULL);

		hListView = CreateWindow(WC_LISTVIEW, L"", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
			220, 10, 860, 400, hWnd, (HMENU)1, hInst, NULL);

		SendMessage(hListView, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_GRIDLINES);

		CreateWindowW(L"STATIC", L"Регионов памяти:", WS_CHILD | WS_VISIBLE,
			220, 420, 150, 20, hWnd, NULL, hInst, NULL);

		hMemoryCountText = CreateWindowW(L"STATIC", L"0", WS_CHILD | WS_VISIBLE,
			360, 420, 100, 20, hWnd, NULL, hInst, NULL);

		CreateWindowW(L"STATIC", L"Минимальный адрес", WS_CHILD | WS_VISIBLE,
			10, 230, 150, 20, hWnd, NULL, hInst, NULL);

		hEditMin = CreateWindow(WC_EDIT, L"0", WS_CHILD | WS_VISIBLE | WS_BORDER,
			10, 250, 150, 20, hWnd, (HMENU)3, hInst, NULL);

		CreateWindowW(L"STATIC", L"Максимальный адрес", WS_CHILD | WS_VISIBLE,
			10, 280, 150, 20, hWnd, NULL, hInst, NULL);

		hEditMax = CreateWindow(WC_EDIT, L"FFFFFFFFFFFFFFF", WS_CHILD | WS_VISIBLE | WS_BORDER,
			10, 300, 150, 20, hWnd, (HMENU)4, hInst, NULL);

		hButtonFilter = CreateWindow(WC_BUTTON, L"Filter", WS_CHILD | WS_VISIBLE,
			10, 330, 80, 20, hWnd, (HMENU)5, hInst, NULL);

		AddListViewColumns(hListView);
		PopulateProcessList(hListBox);
		break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Разобрать выбор в меню:

		if (LOWORD(wParam) == 2 && HIWORD(wParam) == LBN_SELCHANGE) {
			int index = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
			if (index != LB_ERR) {
				selectedProcessID = (DWORD)SendMessage(hListBox, LB_GETITEMDATA, index, 0);

				wchar_t minBuffer[20], maxBuffer[20];
				GetWindowText(hEditMin, minBuffer, 20);
				GetWindowText(hEditMax, maxBuffer, 20);

				LPVOID minAddr = (LPVOID)wcstoll(minBuffer, NULL, 16);
				LPVOID maxAddr = (LPVOID)wcstoll(maxBuffer, NULL, 16);

				PrintMemoryInfo(hListView, selectedProcessID, minAddr, maxAddr);
			}
		}
		else if (LOWORD(wParam) == 5) {
			wchar_t minBuffer[20], maxBuffer[20];
			GetWindowText(hEditMin, minBuffer, 20);
			GetWindowText(hEditMax, maxBuffer, 20);

			LPVOID minAddr = (LPVOID)wcstoll(minBuffer, NULL, 16);
			LPVOID maxAddr = (LPVOID)wcstoll(maxBuffer, NULL, 16);

			PrintMemoryInfo(hListView, selectedProcessID, minAddr, maxAddr);
		}

		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}


	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Добавьте сюда любой код прорисовки, использующий HDC...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void AddListViewColumns(HWND hWndListView) {
	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;

	lvc.pszText = (LPWSTR)L"Base Address";
	lvc.cx = 120;
	ListView_InsertColumn(hWndListView, 1, &lvc);

	lvc.pszText = (LPWSTR)L"Size (Bytes)";
	lvc.cx = 100;
	ListView_InsertColumn(hWndListView, 2, &lvc);

	lvc.pszText = (LPWSTR)L"Type";
	lvc.cx = 100;
	ListView_InsertColumn(hWndListView, 3, &lvc);

	lvc.pszText = (LPWSTR)L"Protect";
	lvc.cx = 100;
	ListView_InsertColumn(hWndListView, 4, &lvc);

	lvc.pszText = (LPWSTR)L"AllocationBase";
	lvc.cx = 120;
	ListView_InsertColumn(hWndListView, 5, &lvc);

	lvc.pszText = (LPWSTR)L"AllocationProtect";
	lvc.cx = 120;
	ListView_InsertColumn(hWndListView, 6, &lvc);

	lvc.pszText = (LPWSTR)L"State";
	lvc.cx = 100;
	ListView_InsertColumn(hWndListView, 7, &lvc);
}

void AddListViewItem(HWND hWndListView, int index, MEMORY_BASIC_INFORMATION mbi) {
	wchar_t buffer[256];
	LVITEM lvi;
	lvi.mask = LVIF_TEXT;
	lvi.iItem = index;
	lvi.iSubItem = 0;

	swprintf_s(buffer, L"%p", mbi.BaseAddress);
	lvi.pszText = buffer;
	ListView_InsertItem(hWndListView, &lvi);

	swprintf_s(buffer, L"%llu", (unsigned long long)mbi.RegionSize);
	ListView_SetItemText(hWndListView, index, 1, buffer);

	swprintf_s(buffer, L"%s", GetTypeString(mbi.Type));
	ListView_SetItemText(hWndListView, index, 2, buffer);

	swprintf_s(buffer, L"%lu", mbi.Protect);
	ListView_SetItemText(hWndListView, index, 3, buffer);

	swprintf_s(buffer, L"%p", mbi.AllocationBase);
	ListView_SetItemText(hWndListView, index, 4, buffer);

	swprintf_s(buffer, L"%s", GetProtectString(mbi.AllocationProtect));
	ListView_SetItemText(hWndListView, index, 5, buffer);

	swprintf_s(buffer, L"%s", GetStateString(mbi.State));
	ListView_SetItemText(hWndListView, index, 6, buffer);
}

void PrintMemoryInfo(HWND hWndListView, DWORD processID, LPVOID minAddress, LPVOID maxAddress) {
	ListView_DeleteAllItems(hWndListView);
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
	if (hProcess == NULL) return;

	MEMORY_BASIC_INFORMATION mbi;
	LPVOID address = minAddress;
	int index = 0;

	while (address < maxAddress && VirtualQueryEx(hProcess, address, &mbi, sizeof(mbi)) == sizeof(mbi)) {
		AddListViewItem(hWndListView, index++, mbi);
		address = (LPVOID)((DWORD_PTR)address + mbi.RegionSize);
	}

	CloseHandle(hProcess);

	wchar_t countBuffer[50];
	swprintf_s(countBuffer, L"%d", index);
	SetWindowText(hMemoryCountText, countBuffer);
}

void PopulateProcessList(HWND hWndListBox) {
	SendMessage(hWndListBox, LB_RESETCONTENT, 0, 0);
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE) return;

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hSnapshot, &pe32)) {
		do {
			wchar_t buffer[256];
			swprintf_s(buffer, L"%s (%d)", pe32.szExeFile, pe32.th32ProcessID);
			int index = SendMessage(hWndListBox, LB_ADDSTRING, 0, (LPARAM)buffer);
			SendMessage(hWndListBox, LB_SETITEMDATA, index, (LPARAM)pe32.th32ProcessID);
		} while (Process32Next(hSnapshot, &pe32));
	}

	CloseHandle(hSnapshot);
}

const wchar_t* GetProtectString(DWORD protect) {
	switch (protect) {
	case PAGE_EXECUTE: return L"PAGE_EXECUTE";
	case PAGE_EXECUTE_READ: return L"PAGE_EXECUTE_READ";
	case PAGE_EXECUTE_READWRITE: return L"PAGE_EXECUTE_READWRITE";
	case PAGE_EXECUTE_WRITECOPY: return L"PAGE_EXECUTE_WRITECOPY";
	case PAGE_NOACCESS: return L"PAGE_NOACCESS";
	case PAGE_READONLY: return L"PAGE_READONLY";
	case PAGE_READWRITE: return L"PAGE_READWRITE";
	case PAGE_WRITECOPY: return L"PAGE_WRITECOPY";
	case PAGE_GUARD: return L"PAGE_GUARD";
	case PAGE_NOCACHE: return L"PAGE_NOCACHE";
	case PAGE_WRITECOMBINE: return L"PAGE_WRITECOMBINE";
	default: return L"-";
	}
}

const wchar_t* GetTypeString(DWORD type) {

    switch (type) {
	case MEM_COMMIT: return L"MEM_COMMIT";
	case MEM_RESERVE: return L"MEM_RESERVE";
	case MEM_DECOMMIT: return L"MEM_DECOMMIT";
	case MEM_RELEASE: return L"MEM_RELEASE";
	case MEM_FREE: return L"MEM_FREE";
	case MEM_PRIVATE: return L"MEM_PRIVATE";
	case MEM_MAPPED: return L"MEM_MAPPED";
	case MEM_IMAGE: return L"MEM_IMAGE";
    default: return L"-";
    }
}

const wchar_t* GetStateString(DWORD state) {
	switch (state) {
	case MEM_COMMIT: return L"MEM_COMMIT";
	case MEM_RESERVE: return L"MEM_RESERVE";
	case MEM_DECOMMIT: return L"MEM_DECOMMIT";
	case MEM_RELEASE: return L"MEM_RELEASE";
	case MEM_FREE: return L"MEM_FREE";
	case MEM_PRIVATE: return L"MEM_PRIVATE";
	case MEM_MAPPED: return L"MEM_MAPPED";
	case MEM_IMAGE: return L"MEM_IMAGE";
	default: return L"-";
	}
}
