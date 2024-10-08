// 2.genshin.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include<graphics.h>
#include<string>
#include<vector>
#pragma comment(lib,"Msimg32.lib")

const int PLAYER_SPEED = 5;
int idx_current_anim = 0;//动画帧索引
const int PLAYER_ANIM_NUM = 6;//动画帧常数总量
const int PLAYER_WIDTH = 80;//玩家宽度
const int PLAYER_HEIGHT = 80;//玩家高度
const int SHADOW_WIDTH = 32;//阴影宽度

IMAGE img_player_left[PLAYER_ANIM_NUM];
IMAGE img_player_right[PLAYER_ANIM_NUM];
POINT Player_pos = { 500,500 };



//重写后的putimage函数，可以利用图片的透明度信息
inline void putimage_alpha(int x, int y, IMAGE* img) {
	int w = img->getwidth();
	int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h, GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });


}

//void LoadAnimation() {
//	for (size_t i = 0; i < PLAYER_ANIM_NUM; i++)
//	{
//		std::wstring path = L"img/player_left_" + std::to_wstring(i) + L".png";
//		loadimage(&img_player_left[i],path.c_str());
//	}
//	for (size_t i = 0; i < PLAYER_ANIM_NUM; i++)
//	{
//		std::wstring path = L"img/player_right_" + std::to_wstring(i) + L".png";
//		loadimage(&img_player_left[i], path.c_str());
//	}
//	
//}

class Animation
{
public:
	Animation(LPCTSTR path, int num, int interval) {
		interval_ms = interval;
		TCHAR path_file[256];
		for (size_t i = 0; i < num; i++)
		{
			_stprintf_s(path_file, path, i);
			IMAGE* frame = new IMAGE();
			loadimage(frame, path_file);
			frame_list.push_back(frame);
		}
	}
	~Animation() {
		for (size_t i = 0; i < frame_list.size(); i++)
		{
			delete frame_list[i];
		}
	}
	void Play(int x,int y, int delta){
		timer += delta;
		if (timer >= interval_ms) {
			idx_frame = (idx_frame + 1) % frame_list.size();
			timer = 0;
		}
		putimage_alpha(x, y, frame_list[idx_frame]);
	}
private:
	int timer = 0;		//动画计时器
	int idx_frame = 0;	//动画帧索引
	int interval_ms = 0;
	std::vector<IMAGE*> frame_list;
};

Animation  anim_left_player(_T("img/player_left_%d.png"), 6, 45);
Animation  anim_right_player(_T("img/player_right_%d.png"), 6, 45);

void DrawPlayer(int delta, int dir_x) {
	int pos_shadow_x = Player_pos.x + (PLAYER_WIDTH / 2 - SHADOW_WIDTH / 2);
	int pos_shadow_y = Player_pos.y + PLAYER_HEIGHT - 8;
	static bool facing_left = false;
	if (dir_x < 0)
		facing_left = true;
	else if (dir_x > 0)
		facing_left = false;
	if (facing_left)
		anim_left_player.Play(Player_pos.x, Player_pos.y, delta);
	else
		anim_right_player.Play(Player_pos.x, Player_pos.y, delta);
}

int main()
{
	initgraph(1280, 720);
	bool running = true;
	bool is_move_up = false;	
	bool is_move_down = false;	
	bool is_move_left = false;	
	bool is_move_right = false;	
	ExMessage msg;
	IMAGE img_background; IMAGE img_shadow;
	//LoadAnimation();
	loadimage(&img_background, _T("img/background.png"));
	loadimage(&img_shadow, _T("img/shadow_player.png"));
	BeginBatchDraw();//	启用双缓冲
	while (running)
	{
		DWORD start_time = GetTickCount();
		while (peekmessage(&msg)) {
			if (msg.message == WM_KEYDOWN)
			{
				switch (msg.vkcode)
				{
				case VK_UP:
					is_move_up = true;
					break;
				case VK_DOWN:
					is_move_down = true;
					break;
				case VK_LEFT:
					is_move_left = true;
					break;
				case VK_RIGHT:
					is_move_right = true;
					break;
				}
			}
			else if(msg.message == WM_KEYUP)
			{
				switch (msg.vkcode)
				{
				case VK_UP:
					is_move_up = false;
					break;
				case VK_DOWN:
					is_move_down = false;
					break;
				case VK_LEFT:
					is_move_left = false;
					break;
				case VK_RIGHT:
					is_move_right = false;
					break;
				}
			}
		}
	if (is_move_up) Player_pos.y -= PLAYER_SPEED;
	if (is_move_down) Player_pos.y += PLAYER_SPEED;
	if (is_move_left) Player_pos.x -= PLAYER_SPEED;
	if (is_move_right) Player_pos.x += PLAYER_SPEED;


		//static int counter = 0;//记录当前动画帧播放了几个游戏帧         

		//if (++counter%5 == 0) {

		//	idx_current_anim++;
		//}

		//idx_current_anim = idx_current_anim % PLAYER_ANIM_NUM;//使动画循环播放

		cleardevice();
		putimage(0, 0, &img_background);
		DrawPlayer(1000 / 144, is_move_right - is_move_left);
		FlushBatchDraw();
		DWORD end_time = GetTickCount();
		DWORD delta_time = end_time - start_time;
		if (delta_time < 1000 / 144) {
			Sleep(1000 / 144 - delta_time);
		}
	}
	EndBatchDraw();
	return 0;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
