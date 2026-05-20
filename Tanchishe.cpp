#include<stdio.h>
#include<time.h>
#include<windows.h>
#include<stdlib.h>
#include<string.h>

#define U 1
#define D 2
#define L 3
#define R 4       //蛇的状态，U：上 ；D：下；L:左 R：右

// 蛇身节点结构
typedef struct SNAKE
{
    int x;
    int y;
    struct SNAKE* next;
}snake;

// ====================== 新增：用户表结构 ======================
typedef struct User {
    int id;             // 用户ID（自增主键）
    char username[20];  // 用户名
    char password[20];  // 密码
} User;

// ====================== 新增：游戏日志结构 ======================
typedef struct GameLog {
    int logId;          // 日志ID（自增主键）
    int userId;         // 关联用户ID（外键）
    char username[20];  // 用户名
    char startTime[50]; // 游戏开始时间
    int duration;       // 游戏持续时长（秒）
    int score;          // 本局得分
} GameLog;

// 全局变量（原有+新增）
int score = 0, add = 10;//总得分与每次吃食物得分
int status, sleeptime = 200;//每次运行的时间间隔
snake* head, * food;//蛇头指针，食物指针
snake* q;//遍历蛇的时候用到的指针
int endgamestatus = 0; //游戏结束的情况，1：撞到墙；2：咬到自己；3：主动退出游戏

// 新增全局变量
User currentUser;      // 当前登录用户信息
time_t gameStartTime;  // 游戏开始时间（用于计算时长）

// 声明全部函数（原有+新增）
void Pos(int x, int y);
void creatMap();
void initsnake();
int biteself();
void createfood();
void cantcrosswall();
void snakemove();
void pause();
void gamecircle();
void welcometogame();
void endgame();
void gamestart();

// 新增函数声明（对应5项拓展要求）
int userRegister();       // (1) 用户注册
int userLogin();          // (2) 用户登录验证
void saveGameLog();       // (5) 保存游戏日志
void showGameLog();       // (4) F5查看游戏日志
int getNextUserId();      // 获取下一个自增用户ID
int getNextLogId();       // 获取下一个自增日志ID
void initUserFile();      // 初始化用户表文件
void initLogFile();       // 初始化日志表文件

// 设置光标位置（原有）
void Pos(int x, int y)
{
    COORD pos;
    HANDLE hOutput;
    pos.X = x;
    pos.Y = y;
    hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(hOutput, pos);
}

// 创建地图（原有，适配显示）
void creatMap()
{
    int i;
    for (i = 0; i < 50; i += 1)
    {
        Pos(i, 0);
        printf("■");
        Pos(i, 20);
        printf("■");
    }
    for (i = 1; i < 20; i++)
    {
        Pos(0, i);
        printf("■");
        Pos(49, i);
        printf("■");
    }
}

// 初始化蛇身（原有，适配显示）
void initsnake()
{
    snake* tail;
    int i;
    tail = (snake*)malloc(sizeof(snake));
    tail->x = 10;
    tail->y = 5;
    tail->next = NULL;
    for (i = 1; i <= 4; i++)
    {
        head = (snake*)malloc(sizeof(snake));
        head->next = tail;
        head->x = 10 + i;
        head->y = 5;
        tail = head;
    }
    while (tail != NULL)
    {
        Pos(tail->x, tail->y);
        printf("■");
        tail = tail->next;
    }
}

// 判断是否咬到自己（原有）
int biteself()
{
    snake* self;
    self = head->next;
    while (self != NULL)
    {
        if (self->x == head->x && self->y == head->y)
        {
            return 1;
        }
        self = self->next;
    }
    return 0;
}

// 创建食物（原有，修复后）
void createfood()
{
    snake* food_1 = (snake*)malloc(sizeof(snake));

    do {
        food_1->x = rand() % 45 + 2;
        food_1->y = rand() % 18 + 2;
    } while (food_1->x == 0 || food_1->x == 49 || food_1->y == 0 || food_1->y == 20);

    q = head;
    int isOverlap = 0;
    while (q != NULL)
    {
        if (q->x == food_1->x && q->y == food_1->y)
        {
            isOverlap = 1;
            break;
        }
        q = q->next;
    }

    if (isOverlap) {
        free(food_1);
        createfood();
        return;
    }

    Pos(food_1->x, food_1->y);
    food = food_1;
    printf("★");
}

// 不能穿墙（原有，适配新地图）
void cantcrosswall()
{
    if (head->x <= 0 || head->x >= 49 || head->y <= 0 || head->y >= 20)
    {
        endgamestatus = 1;
        endgame();
    }
}

// 蛇移动（原有，修复后）
void snakemove()
{
    snake* nexthead;
    cantcrosswall();

    nexthead = (snake*)malloc(sizeof(snake));
    if (status == U)
    {
        nexthead->x = head->x;
        nexthead->y = head->y - 1;
    }
    if (status == D)
    {
        nexthead->x = head->x;
        nexthead->y = head->y + 1;
    }
    if (status == L)
    {
        nexthead->x = head->x - 1;
        nexthead->y = head->y;
    }
    if (status == R)
    {
        nexthead->x = head->x + 1;
        nexthead->y = head->y;
    }

    if (nexthead->x == food->x && nexthead->y == food->y)
    {
        nexthead->next = head;
        head = nexthead;
        q = head;
        while (q != NULL)
        {
            Pos(q->x, q->y);
            printf("■");
            q = q->next;
        }
        score += add;
        createfood();
    }
    else
    {
        nexthead->next = head;
        head = nexthead;
        q = head;
        while (q->next != NULL && q->next->next != NULL)
        {
            Pos(q->x, q->y);
            printf("■");
            q = q->next;
        }
        Pos(q->next->x, q->next->y);
        printf(" ");
        free(q->next);
        q->next = NULL;
        Pos(head->x, head->y);
        printf("■");
    }

    if (biteself() == 1)
    {
        endgamestatus = 2;
        endgame();
    }
}

// 暂停功能（原有）
void pause()
{
    while (1)
    {
        Sleep(300);
        if (GetAsyncKeyState(VK_SPACE))
        {
            break;
        }
    }
}

// 游戏控制核心（修改：新增用户显示、F5日志功能）
void gamecircle()
{
    // ====================== (3) 显示当前游戏用户 ======================
    Pos(55, 3);
    printf("***%s正在游戏中***", currentUser.username);
    // ====================== (4) 显示F5日志提示 ======================
    Pos(55, 4);
    printf("按F5显示游戏用户日志");

    Pos(55, 6);
    printf("得分：%d  ", score);
    Pos(55, 7);
    printf("每个食物得分：%d分", add);
    Pos(55, 9);
    printf("不能穿墙，不能咬到自己");
    Pos(55, 10);
    printf("↑↓←→控制蛇移动");
    Pos(55, 11);
    printf("F1加速 F2减速");
    Pos(55, 12);
    printf("ESC退出 space暂停");

    status = R;
    gameStartTime = time(NULL); // 记录游戏开始时间

    while (1)
    {
        Pos(55, 6);
        printf("得分：%d  ", score);
        Pos(55, 7);
        printf("每个食物得分：%d分", add);

        // 方向/功能键控制
        if (GetAsyncKeyState(VK_UP) && status != D) status = U;
        else if (GetAsyncKeyState(VK_DOWN) && status != U) status = D;
        else if (GetAsyncKeyState(VK_LEFT) && status != R) status = L;
        else if (GetAsyncKeyState(VK_RIGHT) && status != L) status = R;
        else if (GetAsyncKeyState(VK_SPACE)) pause();
        else if (GetAsyncKeyState(VK_ESCAPE)) { endgamestatus = 3; break; }
        else if (GetAsyncKeyState(VK_F1)) {
            if (sleeptime >= 50) { sleeptime -= 30; add += 2; }
        }
        else if (GetAsyncKeyState(VK_F2)) {
            if (sleeptime < 350) { sleeptime += 30; add -= 2; if (add < 1) add = 1; }
        }
        // ====================== (4) F5查看日志功能 ======================
        else if (GetAsyncKeyState(VK_F5))
        {
            showGameLog(); // 显示日志
            // 返回游戏后重绘界面
            system("cls");
            creatMap();
            q = head;
            while (q != NULL) { Pos(q->x, q->y); printf("■"); q = q->next; }
            Pos(food->x, food->y); printf("★");
            // 重绘用户信息和提示
            Pos(55, 3); printf("***%s正在游戏中***", currentUser.username);
            Pos(55, 4); printf("按F5显示游戏用户日志");
            Pos(55, 6); printf("得分：%d  ", score);
            Pos(55, 7); printf("每个食物得分：%d分", add);
            Pos(55, 9); printf("不能穿墙，不能咬到自己");
            Pos(55, 10); printf("↑↓←→控制蛇移动");
            Pos(55, 11); printf("F1加速 F2减速");
            Pos(55, 12); printf("ESC退出 space暂停");
            Sleep(200); // 防止重复触发
        }

        Sleep(sleeptime);
        snakemove();
    }
}

// 欢迎界面（原有）
void welcometogame()
{
    Pos(20, 10);
    printf("===== 贪吃蛇游戏 =====\n");
    Pos(20, 12);
    printf("食物显示为★，蛇身显示为■\n");
    Pos(20, 14);
    printf("按任意键开始游戏...");
    system("pause>nul");
    system("cls");
}

// 结束游戏（修改：新增保存日志）
void endgame()
{
    // ====================== (5) 游戏结束自动保存日志 ======================
    saveGameLog();

    system("cls");
    Pos(20, 10);
    if (endgamestatus == 1) printf("撞到墙了！游戏结束！");
    else if (endgamestatus == 2) printf("咬到自己了！游戏结束！");
    else if (endgamestatus == 3) printf("您主动退出了游戏！");

    Pos(20, 12);
    printf("最终得分：%d\n", score);
    Pos(20, 14);
    printf("按任意键退出...");
    system("pause>nul");
    exit(0);
}

// 初始化用户/日志文件（新增）
void initUserFile() {
    FILE* fp = fopen("user.txt", "a+");
    if (fp == NULL) { printf("用户表文件创建失败！"); exit(1); }
    fclose(fp);
}
void initLogFile() {
    FILE* fp = fopen("log.txt", "a+");
    if (fp == NULL) { printf("日志表文件创建失败！"); exit(1); }
    fclose(fp);
}

// 获取自增ID（新增）
int getNextUserId() {
    FILE* fp = fopen("user.txt", "r");
    if (fp == NULL) return 1;
    int maxId = 0;
    User temp;
    while (fscanf(fp, "%d,%[^,],%s", &temp.id, temp.username, temp.password) != EOF) {
        if (temp.id > maxId) maxId = temp.id;
    }
    fclose(fp);
    return maxId + 1;
}
int getNextLogId() {
    FILE* fp = fopen("log.txt", "r");
    if (fp == NULL) return 1;
    int maxId = 0;
    GameLog temp;
    while (fscanf(fp, "%d,%d,%[^,],%[^,],%d,%d",
        &temp.logId, &temp.userId, temp.username, temp.startTime, &temp.duration, &temp.score) != EOF) {
        if (temp.logId > maxId) maxId = temp.logId;
    }
    fclose(fp);
    return maxId + 1;
}

// ====================== (1) 用户注册功能（新增） ======================
int userRegister() {
    system("cls");
    User newUser;
    printf("===== 新用户注册 =====\n");
    printf("请输入用户名(3-20位)：");
    scanf("%s", newUser.username);
    if (strlen(newUser.username) < 3 || strlen(newUser.username) > 20) {
        printf("用户名长度不符合要求！\n");
        system("pause");
        return 0;
    }

    printf("请输入密码(6-20位)：");
    scanf("%s", newUser.password);
    if (strlen(newUser.password) < 6 || strlen(newUser.password) > 20) {
        printf("密码长度不符合要求！\n");
        system("pause");
        return 0;
    }

    // 检查用户名是否重复
    FILE* fp = fopen("user.txt", "r");
    User temp;
    while (fscanf(fp, "%d,%[^,],%s", &temp.id, temp.username, temp.password) != EOF) {
        if (strcmp(temp.username, newUser.username) == 0) {
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);

    // 写入用户表
    newUser.id = getNextUserId();
    fp = fopen("user.txt", "a+");
    fprintf(fp, "%d,%s,%s\n", newUser.id, newUser.username, newUser.password);
    fclose(fp);
    return 1;
}

// ====================== (2) 用户登录验证（新增） ======================
int userLogin() {
    system("cls");
    char inputUser[20], inputPwd[20];
    printf("===== 用户登录 =====\n");
    printf("请输入用户名：");
    scanf("%s", inputUser);
    printf("请输入密码：");
    scanf("%s", inputPwd);

    FILE* fp = fopen("user.txt", "r");
    User temp;
    while (fscanf(fp, "%d,%[^,],%s", &temp.id, temp.username, temp.password) != EOF) {
        if (strcmp(temp.username, inputUser) == 0 && strcmp(temp.password, inputPwd) == 0) {
            // 登录成功，保存当前用户信息
            currentUser.id = temp.id;
            strcpy(currentUser.username, temp.username);
            strcpy(currentUser.password, temp.password);
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

// ====================== (5) 保存游戏日志（新增） ======================
void saveGameLog() {
    GameLog log;
    log.logId = getNextLogId();
    log.userId = currentUser.id;
    strcpy(log.username, currentUser.username);

    // 格式化开始时间
    time_t t = gameStartTime;
    strcpy(log.startTime, ctime(&t));
    log.startTime[strlen(log.startTime)-1] = '\0'; // 去除换行符

    log.duration = (int)(time(NULL) - gameStartTime); // 计算时长（秒）
    log.score = score;

    // 写入日志表
    FILE* fp = fopen("log.txt", "a+");
    fprintf(fp, "%d,%d,%s,%s,%d,%d\n",
        log.logId, log.userId, log.username, log.startTime, log.duration, log.score);
    fclose(fp);
}

// ====================== (4) 显示游戏日志（新增） ======================
void showGameLog() {
    system("cls");
    printf("===== %s 的游戏用户日志 =====\n", currentUser.username);
    printf("%-6s %-6s %-15s %-25s %-10s %-8s\n",
        "日志ID", "用户ID", "用户名", "开始时间", "时长(秒)", "得分");
    printf("---------------------------------------------------------------\n");

    FILE* fp = fopen("log.txt", "r");
    GameLog temp;
    while (fscanf(fp, "%d,%d,%[^,],%[^,],%d,%d",
        &temp.logId, &temp.userId, temp.username, temp.startTime, &temp.duration, &temp.score) != EOF) {
        // 只显示当前用户的日志
        if (temp.userId == currentUser.id) {
            printf("%-6d %-6d %-15s %-25s %-10d %-8d\n",
                temp.logId, temp.userId, temp.username, temp.startTime, temp.duration, temp.score);
        }
    }
    fclose(fp);

    printf("\n按任意键返回游戏...");
    system("pause>nul");
}

// 游戏初始化（修改：新增登录/注册流程）
void gamestart()
{
    system("mode con cols=80 lines=25");
    srand((unsigned)time(NULL));

    // 初始化用户表和日志表文件
    initUserFile();
    initLogFile();

    // ====================== 登录/注册选择 ======================
    while (1)
    {
        system("cls");
        Pos(30, 10);
        printf("===== 贪吃蛇游戏登录系统 =====\n");
        Pos(30, 12);
        printf("1. 新用户注册\n");
        Pos(30, 13);
        printf("2. 已有账号登录\n");
        Pos(30, 15);
        printf("请输入选项(1/2)：");

        int choice;
        scanf("%d", &choice);

        if (choice == 1) {
            if (userRegister()) {
                Pos(30, 17); printf("注册成功！请登录\n");
                system("pause");
            } else {
                Pos(30, 17); printf("注册失败！用户名已存在\n");
                system("pause");
            }
        } else if (choice == 2) {
            if (userLogin()) {
                Pos(30, 17); printf("登录成功！即将进入游戏...\n");
                Sleep(1000);
                break;
            } else {
                Pos(30, 17); printf("登录失败！用户名或密码错误\n");
                system("pause");
            }
        } else {
            Pos(30, 17); printf("输入错误！请重新选择\n");
            system("pause");
        }
    }

    // 原有游戏初始化流程
    welcometogame();
    creatMap();
    initsnake();
    createfood();
}

// 主函数（原有）
int main()
{
    gamestart();
    gamecircle();
    endgame();
    return 0;
}
