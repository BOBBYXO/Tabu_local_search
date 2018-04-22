
#include <stdio.h>
#include <iostream>
#include<string>
#include<fstream>
#include<sstream>
#include<time.h>
#include<io.h>
#include<ctime>
using namespace std;

const int NUM = 125;  //��������
const int COLOR = 5; //��ɫ��
const string filename = "DSJC125.1.txt";
unsigned SEED;//�������
time_t t_start, t_end;//��ʱ��
time_t c_start, c_end;//��ʱ����
struct tm now_time;

int F = 0;     //��ͻ�� ��
int BEST_F = 20000; //��ʷ��ó�ͻ��
int ITER = 0; //��������
int MAXITER = 1000000;//����������
int relation[NUM][NUM] = {};//������ϵ����
int solution[NUM];//��
int adjacent[NUM][COLOR];//�����
int delt = 0;//��������
int tabu[NUM][COLOR] = {};//���ɱ�
int node;//�������
int color, old_color;//������ɫ ԭ����ɫ
int candidate[NUM][2] = {};//�ǽ��ɺ�ѡ����
int candidate_tabu[NUM][2] = {};//���ɺ�ѡ����

int conflict[NUM] = {};//��ͻ����
int *conflict_index[NUM] = {};//��ͻ������
int conflict_edge = 0;

int GeneIter = 0;//���沽��
void InitF()//��ʼ������
{
	SEED = (unsigned)time(NULL);
	srand(SEED);//�����������
	ifstream inf;
	inf.open(filename);
	string sline;
	string a, b, c;
	int s1, s2;
    
	t_start= time(NULL);
	localtime_s(&now_time, &t_start);
	c_start = clock();//��ȡ��ʼʱ��
	

	while (getline(inf, sline))//��ȡ�ļ��е����� 
	{
		istringstream sin(sline);
		sin >> a >> b >> c;

		if (b != ""&&c != "")
		{
			s1 = stoi(b);//�ַ���ת��
			s2 = stoi(c);
			relation[s1 - 1][s2 - 1] = 1;
			relation[s2 - 1][s1 - 1] = 1;
		}
	}
	inf.close();

	//���ɳ�ʼ��
	for (int i1 = 1; i1 < NUM; i1++)
	{
		solution[i1] = rand() % COLOR;
	}
	int count = 1;//�������ת��Ϊ������ṹ ˳�����������
	int flag = 1;//��ͻ���ʼ����־λ
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
					F++;//���ɳ�ͻ����
					if (flag == 1)
					{
						conflict[conflict_edge] = i;
						conflict_index[i] = &conflict[conflict_edge];
						conflict_edge++;
						flag = 0;//��ʾ�õ��Ѿ�����
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
	conflict[conflict_edge] = -1; //���ó�ͻ����ֵֹ
	F = F / 2;//תΪ��ͻ�Եĸ���
}

void DateNote()
{
	c_end = clock();
	t_end = time(NULL);
	double t1 = difftime(c_end, c_start)/1000;
	double t2 = difftime(t_end, t_start);
	cout << t2 +t1;//��¼����ʱ��

	ofstream ofile;
	if ((_access("DataNote.csv", 0)) == -1)//������ͷ
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
	for (int i = 0; i < NUM; i++)//�жϽ�ĺϷ��� 0Ϊ�Ϸ�
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
	int n_color;//��ǰ��ɫ
	int *adj; //���������ָ��

	int *tabu_p;
	int count = 0;
	int count_tabu = 0;
	int confnode;
	for (int i = 0; i<conflict_edge; i++)
	{
		confnode = conflict[i];//ȡ����ͻ��
		n_color = solution[confnode];//ȡ����ͻ����ɫ
		adj = adjacent[confnode];//ȡ����ͻ���������ָ��
		tabu_p = tabu[confnode];//ȡ���ɱ���ָ��
		for (int j = 0; j < COLOR; j++)//������ɫ
		{
			if (n_color != j)//�ų���ǰ��ɫ
			{
				temp = adj[j] - adj[n_color];//�������� �ƶ�����ƶ�ǰ					
					if (tabu_p[j] <= ITER)//�ж��Ƿ��ڽ���
					{
						if (temp <= delt)
						{
							if (temp < delt)//�ǽ���
							{
								delt = temp;
								count = 0;
							}
							candidate[count][0] = confnode;//��ѡ���
							candidate[count][1] = j;//��Ӧ��ѡ��ɫ
							count++;
						}
					}
				
				else {
					if (temp <= delt_tabu)
					{
						if (temp < delt_tabu)//����
						{
							delt_tabu = temp;
							count_tabu = 0;
						}
						candidate_tabu[count_tabu][0] = confnode;//��ѡ���
						candidate_tabu[count_tabu][1] = j;//��Ӧ��ѡ��ɫ
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
		color = candidate[temp][1];//���ѡ���ƶ�
	}
}
void Makemove()
{
	int i = 0, j = 0, k = 0;
	old_color = solution[node];
	solution[node] = color;//�����ƶ�
	F += delt;//���³�ͻ��
	if (F < BEST_F)
	{
		BEST_F = F;
		//cout << BEST_F << '\t' << ITER << endl;
	}
	tabu[node][old_color] = ITER + F + rand() % 10;//������ɱ�

	if (adjacent[node][color] == 0 && conflict_index[node] != NULL)//��ͻ��Ϊ0
	{
		conflict_edge--;//��ͻ���ȼ���
		j = conflict[conflict_edge];
		*conflict_index[node] = j;
		conflict_index[j] = conflict_index[node];
		conflict[conflict_edge] = -1;
		conflict_index[node] = NULL;
		
	}
	if (adjacent[node][color] > 0 && conflict_index[node] == NULL)//�³�ͻ��
	{
		conflict[conflict_edge] = node;
		conflict_index[node] =& conflict[conflict_edge];
		conflict_edge++;//��ͻ��������
		conflict[conflict_edge] = -1;
	}


	int *node_p = relation[node];
	int *adj;
	int temp;
	int rel_color;
	while (1)//���������
	{
		temp = node_p[i];//�й�ϵ�ĵ�
		if (temp == -1) break;
		i++;
		adj = adjacent[temp];
		adj[old_color]--;
		adj[color]++;
		rel_color = solution[temp];//�õ���ɫ

		if (adj[rel_color] == 0 && conflict_index[temp] != NULL)//��ͻ��Ϊ0
		{
			conflict_edge--;//��ͻ���ȼ���
			j = conflict[conflict_edge];
			*conflict_index[temp] = j;
			conflict_index[j] = conflict_index[temp];
			conflict[conflict_edge] = -1;
			conflict_index[temp] = NULL;

		}
		if (adj[rel_color] > 0 && conflict_index[temp] == NULL)//�³�ͻ��
		{
			conflict[conflict_edge] = temp;
			conflict_index[temp] = &conflict[conflict_edge];
			conflict_edge++;//��ͻ��������
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