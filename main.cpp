
#include <stdio.h>
#include <iostream>
#include<string>
#include<fstream>
#include<sstream>
#include<time.h>
#include<io.h>
#include<ctime>
using namespace std;

const int NUM = 125;  //算例点数
const int COLOR = 5; //颜色数
const string filename = "DSJC125.1.txt";
unsigned SEED;//随机种子
time_t t_start, t_end;//计时秒
time_t c_start, c_end;//计时毫秒
struct tm now_time;

int F = 0;     //冲突对 数
int BEST_F = 20000; //历史最好冲突数
int ITER = 0; //迭代步数
int MAXITER = 1000000;//最大迭代步数
int relation[NUM][NUM] = {};//点与点关系矩阵
int solution[NUM];//解
int adjacent[NUM][COLOR];//邻域表
int delt = 0;//动作增量
int tabu[NUM][COLOR] = {};//禁忌表
int node;//动作结点
int color, old_color;//动作颜色 原本颜色
int candidate[NUM][2] = {};//非禁忌候选动作
int candidate_tabu[NUM][2] = {};//禁忌候选动作

int conflict[NUM] = {};//冲突结点表
int *conflict_index[NUM] = {};//冲突点索引
int conflict_edge = 0;

int GeneIter = 0;//交叉步数
void InitF()//初始化函数
{
	SEED = (unsigned)time(NULL);
	srand(SEED);//更改随机种子
	ifstream inf;
	inf.open(filename);
	string sline;
	string a, b, c;
	int s1, s2;
    
	t_start= time(NULL);
	localtime_s(&now_time, &t_start);
	c_start = clock();//获取起始时间
	

	while (getline(inf, sline))//读取文件中的数据 
	{
		istringstream sin(sline);
		sin >> a >> b >> c;

		if (b != ""&&c != "")
		{
			s1 = stoi(b);//字符串转整
			s2 = stoi(c);
			relation[s1 - 1][s2 - 1] = 1;
			relation[s2 - 1][s1 - 1] = 1;
		}
	}
	inf.close();

	//生成初始解
	for (int i1 = 1; i1 < NUM; i1++)
	{
		solution[i1] = rand() % COLOR;
	}
	int count = 1;//将大矩阵转换为索引表结构 顺便生成邻域表
	int flag = 1;//冲突表初始化标志位
	for (int i = 0; i < NUM; i++)
	{
		count = 0;
		flag = 1;

		for (int j = 0; j < NUM; j++)
		{
			if (relation[i][j] == 1)
			{
				relation[i][count] = j;
				count++;
				adjacent[i][solution[j]]++;
				if (solution[i] == solution[j])
				{
					F++;//生成冲突总数
					if (flag == 1)
					{
						conflict[conflict_edge] = i;
						conflict_index[i] = &conflict[conflict_edge];
						conflict_edge++;
						flag = 0;//表示该点已经进表
					}
				}
			}
			if (j == NUM - 1)
			{
				relation[i][count] = -1;
			}
			if (flag == 1) conflict_index[i] = NULL;

		}
	}
	conflict[conflict_edge] = -1; //设置冲突表终止值
	F = F / 2;//转为冲突对的个数
}

void DateNote()
{
	c_end = clock();
	t_end = time(NULL);
	double t1 = difftime(c_end, c_start)/1000;
	double t2 = difftime(t_end, t_start);
	cout << t2 +t1;//记录运行时间

	ofstream ofile;
	if ((_access("DataNote.csv", 0)) == -1)//创建表头
	{
		ofile.open("DataNote.csv", ios::out | ios::app);
		ofile << "Data,Instance,Algorithm,RandSeed,IterCount,Duration,GenerationCount,Legality,Deviation,Optima,Solution" ;
	}
	else 
	{
		ofile.open("DataNote.csv", ios::out | ios::app);
	}
	ofile << endl;
	ofile<<now_time.tm_year + 1900<<'-'<< now_time.tm_mon + 1<<'-'<<now_time.tm_mday<<' '
		<< now_time.tm_hour<<':'<< now_time.tm_min<<':'<< now_time.tm_sec <<',';
	ofile << filename << ',' << "Tabu Local Search," << SEED << ','<<ITER<<','<<t1+t2<<','<<GeneIter<<',';

	int test = 0,test_color=0;
	for (int i = 0; i < NUM; i++)//判断解的合法性 0为合法
	{
		test_color = solution[i];
		if (adjacent[i][test_color] != 0)test++;
	}

	ofile << test << ',' << BEST_F << ',' << COLOR<<',';
	for (int i = 0; i < NUM; i++) 
	{
		ofile << solution[i] << ' ';
	}
		
	ofile.close();
}
void Findmove()
{
	delt = 200;
	int delt_tabu = 200;
	int temp;
	int n_color;//当前颜色
	int *adj; //邻域表行首指针

	int *tabu_p;
	int count = 0;
	int count_tabu = 0;
	int confnode;
	for (int i = 0; i<conflict_edge; i++)
	{
		confnode = conflict[i];//取出冲突点
		n_color = solution[confnode];//取出冲突点颜色
		adj = adjacent[confnode];//取出冲突点邻域表首指针
		tabu_p = tabu[confnode];//取禁忌表首指针
		for (int j = 0; j < COLOR; j++)//遍历颜色
		{
			if (n_color != j)//排除当前颜色
			{
				temp = adj[j] - adj[n_color];//计算增量 移动后减移动前					
					if (tabu_p[j] <= ITER)//判断是否在禁忌
					{
						if (temp <= delt)
						{
							if (temp < delt)//非禁忌
							{
								delt = temp;
								count = 0;
							}
							candidate[count][0] = confnode;//候选结点
							candidate[count][1] = j;//对应候选颜色
							count++;
						}
					}
				
				else {
					if (temp <= delt_tabu)
					{
						if (temp < delt_tabu)//禁忌
						{
							delt_tabu = temp;
							count_tabu = 0;
						}
						candidate_tabu[count_tabu][0] = confnode;//候选结点
						candidate_tabu[count_tabu][1] = j;//对应候选颜色
						count_tabu++;
					}
				}
			}
		}
	}

	if (delt_tabu < delt && (delt_tabu < BEST_F - F))
	{
		temp = rand() % count_tabu;
		node = candidate_tabu[temp][0];
		color = candidate_tabu[temp][1];
		delt = delt_tabu;
	}
	else if (count != 0) {
		temp = rand() % count;
		node = candidate[temp][0];
		color = candidate[temp][1];//随机选择移动
	}
}
void Makemove()
{
	int i = 0, j = 0, k = 0;
	old_color = solution[node];
	solution[node] = color;//作出移动
	F += delt;//更新冲突数
	if (F < BEST_F)
	{
		BEST_F = F;
		//cout << BEST_F << '\t' << ITER << endl;
	}
	tabu[node][old_color] = ITER + F + rand() % 10;//进入禁忌表

	if (adjacent[node][color] == 0 && conflict_index[node] != NULL)//冲突数为0
	{
		conflict_edge--;//冲突表长度减少
		j = conflict[conflict_edge];
		*conflict_index[node] = j;
		conflict_index[j] = conflict_index[node];
		conflict[conflict_edge] = -1;
		conflict_index[node] = NULL;
		
	}
	if (adjacent[node][color] > 0 && conflict_index[node] == NULL)//新冲突点
	{
		conflict[conflict_edge] = node;
		conflict_index[node] =& conflict[conflict_edge];
		conflict_edge++;//冲突表长度增加
		conflict[conflict_edge] = -1;
	}


	int *node_p = relation[node];
	int *adj;
	int temp;
	int rel_color;
	while (1)//更新邻域表
	{
		temp = node_p[i];//有关系的点
		if (temp == -1) break;
		i++;
		adj = adjacent[temp];
		adj[old_color]--;
		adj[color]++;
		rel_color = solution[temp];//该点颜色

		if (adj[rel_color] == 0 && conflict_index[temp] != NULL)//冲突数为0
		{
			conflict_edge--;//冲突表长度减少
			j = conflict[conflict_edge];
			*conflict_index[temp] = j;
			conflict_index[j] = conflict_index[temp];
			conflict[conflict_edge] = -1;
			conflict_index[temp] = NULL;

		}
		if (adj[rel_color] > 0 && conflict_index[temp] == NULL)//新冲突点
		{
			conflict[conflict_edge] = temp;
			conflict_index[temp] = &conflict[conflict_edge];
			conflict_edge++;//冲突表长度增加
			conflict[conflict_edge] = -1;
		}
		

	}

}
int main()
{
	InitF();
	cout << F << endl;
	while (BEST_F>0)//ITER < MAXITER &&
	{
		Findmove();
		Makemove();
		ITER++;
	}
    //DateNote();

	//system("pause");
	return 0;
}