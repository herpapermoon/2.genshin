// 2.genshin.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include<graphics.h>
#include<string>
#include<vector>
#include<windows.h>
#pragma comment(lib,"Msimg32.lib")
#pragma comment(lib,"MSIMG32.lib")

int idx_current_anim = 0;//动画帧索引
const int PLAYER_ANIM_NUM = 6;//动画帧常数总量
const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 720;

//重写后的putimage函数，可以利用图片的透明度信息
inline void putimage_alpha(int x, int y, IMAGE* img) {
	int w = img->getwidth();
	int h = img->getheight();
	AlphaBlend(GetImageHDC(NULL), x, y, w, h, GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });


}
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

class Player
{
private:
	const int SPEED = 10;
	const int FRAME_WIDTH = 80;
	const int FRAME_HEIGHT = 80;
	const int SHADOW_WIDTH = 32;
	const int PLAYER_WIDTH = 80;//玩家宽度
	const int PLAYER_HEIGHT = 80;//玩家高度

	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	POINT position = { 500,500 };
	bool is_move_up = false;
	bool is_move_down = false;
	bool is_move_left = false;
	bool is_move_right = false;
public:
	Player() {

		loadimage(&img_shadow, _T("img/shadow_player.png"));
		anim_left = new Animation(_T("img/player_left_%d.png"), 6, 45);
		anim_right = new Animation(_T("img/player_right_%d.png"), 6, 45);
	}
	~Player() {
		delete anim_left;
		delete anim_right;
	}
	void move() {
		int dir_x = is_move_right - is_move_left;
		int dir_y = is_move_down - is_move_up;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(SPEED * normalized_x);
			position.y += (int)(SPEED * normalized_y);
		}
		if (position.x < 0)position.x = 0;
		if (position.y < 0)position.y = 0;
		if (position.x + FRAME_WIDTH > WINDOW_WIDTH) position.x = WINDOW_WIDTH - FRAME_WIDTH;
		if (position.y + FRAME_HEIGHT > WINDOW_HEIGHT) position.y = WINDOW_HEIGHT - FRAME_HEIGHT;
	};
	void Draw(int delta) {
		int pos_shadow_x = position.x + (PLAYER_WIDTH / 2 - SHADOW_WIDTH / 2);
		int pos_shadow_y = position.y + PLAYER_HEIGHT - 8;
		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);

		static bool facing_left = false;
		int dir_x = is_move_right - is_move_left;
		if (dir_x < 0)
			facing_left = true;
		else if (dir_x > 0)
			facing_left = false;

		if (facing_left)
			anim_left->Play(position.x, position.y, delta);
		else
			anim_right->Play(position.x, position.y, delta);

	}

	void ProcessEvent(const ExMessage& msg)
	{
		switch (msg.message)
		{
		case WM_KEYDOWN:
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
			break;
		case WM_KEYUP:
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
	const POINT& GetPosition()const {
		return position;
	}
	const int GetFRAME_WIDTH()const {
		return FRAME_WIDTH;
	}
	const int GetFRAME_HEIGHT() const{
		return FRAME_HEIGHT;
	}
};

class Bullet {
private:
	const int RADIUS = 10;

public:
	POINT position = { 0,0 };

public:
	Bullet() = default;
	~Bullet() = default;
	void Draw() const {
		setlinecolor(RGB(255, 155, 50));
		setfillcolor(RGB(200, 75, 10));
		fillcircle(position.x, position.y, RADIUS);
	}
};
class Enemy
{
private:
	const int SPEED = 2;
	const int FRAME_WIDTH = 80;//敌人宽度
	const int FRAME_HEIGHT = 80;//敌人高度
	const int SHADOW_Width = 48;//阴影宽度

private:
	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	POINT position = { 0,0 };
	bool facing_left = false;
	bool alive = true;
public:
	Enemy() 
	{
		loadimage(&img_shadow, _T("img/shadow_enemy.png"));
		anim_left = new Animation(_T("img/enemy_left_%d.png"), 6, 45);
		anim_right = new Animation(_T("img/enemy_right_%d.png"), 6, 45);

		//敌人生成边界
		enum class SpawnEdge {
			Up = 0,
			Down,
			Left,
			Right
		};
		//将敌人放置在地图外边界处的随机位置
		SpawnEdge edge = (SpawnEdge)(rand() % 4);
		switch (edge)
		{
		case SpawnEdge::Up:
			position.x = rand() % WINDOW_WIDTH;
			position.y = -FRAME_HEIGHT;
			break;
		case SpawnEdge::Down:
			position.x = rand() % WINDOW_WIDTH;
			position.y = WINDOW_HEIGHT;
			break;
		case SpawnEdge::Left:
			position.x = -FRAME_WIDTH;
			position.y = rand() % WINDOW_HEIGHT;
			break;
		case SpawnEdge::Right:
			position.x = WINDOW_WIDTH;
			position.y = rand() % WINDOW_HEIGHT;
			break;
		default:
			break;
		}

	}
	~Enemy() {
		delete anim_left;
		delete anim_right;

	}
	bool CheckBulletCollision(const Bullet& bullet) 
	{
		bool is_overlap_x = bullet.position.x >= position.x && bullet.position.x <= position.x + FRAME_WIDTH;
		bool is_overlap_y = bullet.position.y >= position.y && bullet.position.y <= position.y + FRAME_HEIGHT;
		return is_overlap_x&&is_overlap_y;
	}
	bool CheckPlayerCollision(const Player& myplayer) 
	{
		//将敌人中心位置等效为点，判断点是否在玩家矩形内
		POINT check_position = { position.x + FRAME_WIDTH / 2,position.y + FRAME_HEIGHT / 2 };
		POINT player_position = myplayer.GetPosition();
		bool is_overlap_x = check_position.x >= player_position.x && check_position.x <= player_position.x + myplayer.GetFRAME_WIDTH();
		bool is_overlap_y = check_position.y >= player_position.y && check_position.y <= player_position.y + myplayer.GetFRAME_HEIGHT();
		return is_overlap_x && is_overlap_y;
	}
	void Move(const Player& player)
	{
		const POINT& player_position = player.GetPosition();
		int dir_x = player_position.x - position.x;
		int dir_y = player_position.y - position.y;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0)
		{
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			position.x += (int)(SPEED * normalized_x);
			position.y += (int)(SPEED * normalized_y);
		}
		if (dir_x < 0) facing_left = true;
		else if (dir_x > 0) facing_left = false;
	}
	void Draw(int delta) 
	{
		int pos_shadow_x = position.x + (FRAME_WIDTH / 2 - SHADOW_Width / 2);
		int pos_shadow_y = position.y + FRAME_HEIGHT - 35;
		putimage_alpha(pos_shadow_x, pos_shadow_y, &img_shadow);
		if (facing_left) {
			anim_left->Play(position.x, position.y, delta);
		}
		else
		{
			anim_right->Play(position.x, position.y, delta);
		}

	}
	void Hurt() {
		alive = false;
	}
	bool CheckAlive() {
		return alive;
	}
};
//生成新的敌人
void TryGenerateEnemy(std::vector<Enemy*>& enemy_list) {
	const int INTERVAL = 100;
	static int counter = 0;
	if ((++counter) % INTERVAL == 0)
		enemy_list.push_back(new Enemy());
}
void UpdateBullets(std::vector<Bullet>& bullet_list, const Player& player) {
	const double RADIAL_SPEED = 0.0045;//径向波动速度
	const double TANGENT_SPEED = 0.0055;//切向波动速度
	double radian_interval = 2 * 3.14159 / bullet_list.size();//子弹之间的弧度间隔
	POINT player_position = player.GetPosition();
	double radius = 100 + 25 * sin(GetTickCount() * RADIAL_SPEED);
	for (size_t i = 0; i < bullet_list.size(); i++) {
		double radian = GetTickCount() * TANGENT_SPEED + radian_interval * i;//当前子弹所在弧度值
		bullet_list[i].position.x = player_position.x + player.GetFRAME_WIDTH() / 2 + (int)(radius * sin(radian));
		bullet_list[i].position.y = player_position.y + player.GetFRAME_HEIGHT() / 2 + (int)(radius * cos(radian));


	}
	}
//绘制玩家得分
void DrawPlayerScore(int score)
{
	static TCHAR text[64];
	_stprintf_s(text, _T("当前玩家得分：%d"), score);
	setbkmode(TRANSPARENT);
	settextcolor(RGB(255, 85, 185));
	outtextxy(10, 10, text);
}

int main()
{
	initgraph(1280, 720);
	mciSendString(_T("open mus/bgm.mp3 alias bgm"), NULL, 0, NULL);
	mciSendString(_T("open mus/hit.wav alias hit"), NULL, 0, NULL);
	mciSendString(_T("play bgm repeat from 0"), NULL, 0, NULL);
	bool running = true;
	int score = 0;
	ExMessage msg;IMAGE img_background; Player myplayer;
	std::vector<Enemy*>enemy_list; std::vector<Bullet> bullet_list(3);
	loadimage(&img_background, _T("img/background.png"));
	BeginBatchDraw();//	启用双缓冲
	while (running)
	{
		DWORD start_time = GetTickCount();
		while (peekmessage(&msg)) {
			myplayer.ProcessEvent(msg);
		}
		myplayer.move();
		UpdateBullets(bullet_list, myplayer);
		TryGenerateEnemy(enemy_list);
		for (Enemy* enemy: enemy_list)
		{
			enemy->Move(myplayer);
		}

		//检测敌人和玩家的碰撞
		for (Enemy* enemy : enemy_list)
		{
			if (enemy->CheckPlayerCollision(myplayer))
			{
				static TCHAR text[128];
				_stprintf_s(text, _T("final score:%d !"), score);
				MessageBox(GetHWnd(), _T("扣1看战败cg"), _T("游戏结束"), MB_OK);
				running = false;
				break;
			}
		}
		//检测子弹和敌人的碰撞

		for (Enemy* enemy : enemy_list) {

			for (const Bullet& bullet : bullet_list)
			{
				if (enemy->CheckBulletCollision(bullet)) {
					mciSendString(_T("play hit from 0"), NULL, 0, NULL);																				
					enemy->Hurt();
					score++;
				}
			}
		}
		//移除生命值归零的敌人
		for (size_t i = 0; i < enemy_list.size(); i++) {
			Enemy* enemy = enemy_list[i];
			if (!enemy->CheckAlive()) {
				std::swap(enemy_list[i], enemy_list.back());
				enemy_list.pop_back();
				delete enemy;
			}
		}
		cleardevice();

		putimage(0, 0, &img_background);

		myplayer.Draw(1000 / 144);

		for (Enemy* enemy : enemy_list)
		{
			enemy->Draw(1000/144);
		}

		for (const Bullet& bullet : bullet_list) {
			bullet.Draw();
		}
		DrawPlayerScore(score);
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
