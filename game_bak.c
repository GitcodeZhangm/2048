

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h> 
#include <stdio.h>
#include <linux/input.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <math.h>

#define BGCOLOR 0X00696969 //������ɫ
#define WIDTH 100  //������
#define ROW 4   //����
#define COLUMN 4  //����
#define GAP 5  //�������
#define BEGIN_X 190 //��Ϸ����ʼ������
#define BEGIN_Y 30  //��Ϸ����ʼ������

void draw_point(int x,int y,int color);//����
void daw_mtrix(int x ,int y,int row,int column);//����Ϸ����
void clear();//����
void draw_BMP(int x0,int y0,const char *bmpname);//����ϷͼƬ
void draw_over_BMP();//��game_over ͼƬ
//const char* getBmpBaseNum(int n);//�������ֻ�ȡͼƬ·��
enum finger_move get_finger_move_direction();//��ȡ�������� 
void add_random_num();//�����λ�� ��������(2 ��4)
void init_game();  //��ʼ��
void begin_game();  //����һ����Ϸ


void move_up();//�����ϻ�����
void move_down();//�����»�����
void move_right();//�����һ�����
void move_left();//�����»�����
bool check_over();//����Ƿ������Ϸ

int *p=NULL;
int *plcd;
bool merge=false;//ȫ�ֱ�������¼û�λ����Ƿ�����ϲ�

//ȫ�����飬��¼��ǰÿ�������������ֵ
int nums[4][4]=
{
	0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 ,
	0 , 0 , 0 , 0 ,               
	0 , 0 , 0 , 0 
};

//ö�����ͣ���¼��ָ�����ķ���
enum finger_move
{
	MOVE_UP = 1,//��
	MOVE_DOWN=2,//��
	MOVE_LEFT=3,//��
	MOVE_RIGHT=4,//��
};
	
void main()
{
	int fd=open("/dev/fb0",O_RDWR);
	if(fd<=0)
	{
		perror("error: ");
		return;
	}
	
	p=mmap(NULL,480*800*4, PROT_WRITE,MAP_SHARED,fd, 0);
	plcd=p;
	
	init_game();
	begin_game();
	
	sleep(1);
	clear();
	munmap(p,400*800*4);
	close(fd);
}


//��ʼ��Ϸ
void begin_game()
{
	
	while(!check_over())
	{
		enum finger_move mv = get_finger_move_direction();
		if(mv==MOVE_UP)
			move_up();
		else if (mv==MOVE_LEFT)
			move_left();	
		else if (mv==MOVE_DOWN)
			move_down();
		else if (mv==MOVE_RIGHT)
			move_right();

		if(merge)
		{
			add_random_num();
			
			daw_mtrix(BEGIN_X,BEGIN_Y,ROW,COLUMN);
		}
	}
	draw_over_BMP();
}

/*
����:��ȡ��ָ�ƶ�����
����ֵ:ö�����ͣ������ĸ�����
*/
enum finger_move get_finger_move_direction()
{
	int fd;
	enum finger_move mv;

	//fd = open("/dev/event0", O_RDONLY);
	fd = open("/dev/input/event0", O_RDONLY);
	if (fd == -1)
	{
		perror("open event0 error!");
	}

	struct input_event ev;
	int x1, y1; //���������е�һ���������ֵ
	int x2,y2; //�������������һ���������ֵ

	x1 = -1; 
	y1 = -1;
	
	while (1)
	{
		int r = read(fd, &ev, sizeof(ev));
		if (r == sizeof(ev))
		{
			if (ev.type == EV_ABS && ev.code == ABS_X)
			{
				if (x1 == -1)
					x1 = ev.value;
				x2 = ev.value;
			}

			if (ev.type == EV_ABS && ev.code == ABS_Y)
			{
				if (y1 == -1)
					y1 = ev.value;
				y2 = ev.value;
			}

			//��ָ����
			if (ev.type == EV_KEY && ev.code == BTN_TOUCH && ev.value == 0)
			//if (ev.type == EV_ABS && ev.code == ABS_PRESSURE && ev.value == 0)
			{
				//��ָ�������Ϸ������
				if(x1<180||x1>600||y1<30||y1>450)
				{
					x1=-1;
					y1=-1;
				}

				if(x2<190)
				{
					x2=190;
				}
				if(x2>605)
				{
					x2=605;
				}
				if(y2<30)
				{
					y2=30;
				}
				if(y2>445)
				{
					y2=445;
				}
				
				
				int delta_x = abs(x2-x1);
				int delta_y = abs(y2-y1);

				if (delta_x >2*delta_y)
				{
					if ((x2 > x1)&&delta_x>80)
					{
						mv = MOVE_RIGHT;
					}
					else if((x1>x2)&&delta_x>80)
					{
						mv = MOVE_LEFT;
					}
					break;
					
				} else if (delta_y > 2 * delta_x)
				{
					if ((y2 > y1)&&delta_y>80)
					{
						mv = MOVE_UP;
					}
					else if((y1 > y2)&&delta_y>80)
					{
						mv = MOVE_DOWN;
					}
					break;
				}
				else //�ж�Ϊ����ʧЧ
				{
					x1 = -1;
					y1 = -1;
				}
			}
		}
	
	}

	
	close(fd);

	return mv;
}

/*
	����:�����λ�� ��������(2 ��4)
	����ֵ:��
*/
void add_random_num()
{
	int flag =0,pos=0;
	srand( (int)time(NULL));
	int x =0 ,y = 0;
	int r[16];
	int c[16];
	int i,j,k = 0;
	/*�ж���Щλ��Ϊ��*/
	for(i=0;i<4;i++)
	{
    		for(j=0;j<4;j++)
	    	{
	        	if(nums[i][j]==0)
	        	{
	            		r[k]=i;//�����k+1���ո��x����
	            		c[k]=j;
	            		k++;//�����λ�ĸ���
	        	}
	    	}
	}
	if(k!=0)//�п�λ
	{
    	pos = rand()%k;//һ��k���ո����ѡ��һ����//rand()%k�ķ�Χ[0 ,k-1]
		x = r[pos];//��pos+1���ո��x����
		y = c[pos];
		nums[x][y] = rand()%20>1?2:4;
	}
}

/*
	����:�����Ļ
	����ֵ:��
*/
void clear()
{
	//p=plcd;
	int x ,y;
	for(y=0;y<480;y++)
	{
		for(x=0;x<800;x++)
		{
			draw_point( x, y, BGCOLOR);
		}
	}
	
}

/*
    ����:	��ָ��λ�û�һ��ָ����ɫ�ĵ�
    ����:	x,y: ����; color: ��ɫֵ
    ����ֵ:��
*/
void draw_point(int x,int y,int color)
{
	p=plcd;
	*(p+y*800+x)=color;
}



/*
	����:	����Ϸ��������
	����:  x,y:�������Ͻǵ�����; row.column:������Ŀ
	����ֵ:��
*/
void daw_mtrix(int x ,int y,int row,int column)//��16��bmpͼƬ->��Ϸ����
{
	int m,n; 
	char bmpPath[10] = {0};
		
	for(n=0;n<column;n++)
	{
		for(m=0;m<row;m++)
		{
			//drawBMP(n*(WIDTH+GAP)+x,m*(WIDTH+GAP)+y, getBmpBaseNum(nums[m][n]));//���������д���
			//     ���ӵ�����								��ȡͼƬ
			sprintf(bmpPath,"%d.bmp",nums[m][n]);
			//strcat(bmpPath,".bmp");
			draw_BMP(n*(WIDTH+GAP)+x,m*(WIDTH+GAP)+y,bmpPath);
		}
	}
		
}

/*
	����: ��ͼƬ
	����: x,y:ͼƬ��λ�ã�*bmpname :ͼƬ·��
	����ֵ:��
*/
void draw_BMP(int x0,int y0,const char *bmpname)//��һ��bmpͼƬ
{
	int  fd;
	unsigned char pixs[100*100*3];
	int i = 0;

	fd = open(bmpname, O_RDONLY);
	if (fd == -1)
	{
		perror("open failed:");
		return ;
	}

	lseek(fd, 54, SEEK_SET);
	read(fd, pixs, 100*100*3);
	close(fd);

	int x, y;
	
	for (y = 0; y < 100; y++)
	{
		//��һ��
		for (x = 0; x < 100; x++)
		{
			unsigned char b,g,r;
			int color;
			b = pixs[i++];
			g = pixs[i++];
			r = pixs[i++];

			color = (r << 16) | (g << 8) | b;
			draw_point(x0 + x, y0 + 99 - y, color);
		}
	}
	
}

/*
	����:���ݴ�����������n ��ö�Ӧ��ͼƬ��·��
	����: n :��ǰn ��ֵ����ֵ�ض�Ϊ2  ��n  �η�
	����ֵ: ��ֵn  ��Ӧ��ͼƬ��·��

const char* getBmpBaseNum(int n)
{
	const char* bmpPaths[17]=
	{
		"0.bmp",		"2.bmp",		"4.bmp",
		"8.bmp",		"16.bmp",	"32.bmp",
		"64.bmp",	"128.bmp",	"256.bmp",
		"512.bmp",   	"1024.bmp",   "2048.bmp",
		"4096.bmp",	"8192.bmp", 	"16384.bmp",
		"32768.bmp", "65535.bmp"
	};
	
	//���ö��������������n  ��bmps[]�����ж�Ӧ��bmpPath  λ��
	int index=(int)(log10(n)/log10(2));
	//����·��
	return bmpPaths[index];	
}
*/


/*
	����:��ʼ��
	 ����ֵ:��
*/
void init_game()
{
	clear();
	memset(nums,0,sizeof(nums));     //����������
	add_random_num();      	//��������
	add_random_num();
	daw_mtrix(BEGIN_X,BEGIN_Y,ROW,COLUMN);		//������Ϸ�Ŀ�ʼ�����Լ�����������
}

/*
	����:����Ϸ����ʱ��ͼƬ
	����ֵ:��
*/
void draw_over_BMP()
{
	int  fd;
	unsigned char pixs[480*800*3];
	int i = 0;

	fd = open("game_over.bmp", O_RDONLY);
	if (fd == -1)
	{
		perror("open failed:");
		return ;
	}

	lseek(fd, 54, SEEK_SET);
	read(fd, pixs, 480*800*3);
	close(fd);

	int x, y;
	for (y = 0; y < 480; y++)
	{
		//��һ��
		for (x = 0; x < 800; x++)
		{
			unsigned char b,g,r;
			int color;
			b = pixs[i++];
			g = pixs[i++];
			r = pixs[i++];

			color = (r << 16) | (g << 8) | b;
			draw_point( x,479 - y, color);
		}
	}
		
}

/*
	 ����:�ϻ�ʱ�߼�����
	 ����ֵ:��
*/
void move_up() 
{
	merge=false;
	int i,j,k;
	for(i=0;i<4;i++)//4�У�����ѭ���Ĵ�
	{
		for(j=0;j<4;j++)//����һ��
		{
			for(k=j+1;k<4;k++)
			{
				if(nums[k][i]>0)//
				{
					if(nums[j][i]==0)
					{
							nums[j][i]=nums[k][i];
							nums[k][i]=0;
							merge=true;
					}
					else if(nums[j][i]==nums[k][i])
					{
							nums[j][i]=(nums[j][i]*2);
							nums[k][i]=0;
							merge=true;
					}
					break;
				}
			}
			}
	}
}

//�»�
void move_down()
{
	merge=false;
	int y,x,x1;
	for(y=0;y<4;y++)
	{
    	for(x=3;x>=0;x--)
		{
			for(x1=x-1;x1>=0;x1--)
			{
        		if(nums[x1][y]>0)
				{
	    			if(nums[x][y]<=0)
					{
            			nums[x][y]=(nums[x1][y]);
            			nums[x1][y]=0;
            			x++;
            			merge=true;
        			}else if(nums[x][y]==nums[x1][y])
        			{
            			nums[x][y]=(nums[x][y]*2);
            			nums[x1][y]=0;
            			merge=true;
        			}
        			break;
    			}
    		}
       	}
    }
}

//��
void move_left()  
{
	merge=false;
   	int x,y,y1;
    for(x=0;x<4;x++)
	{
      	for(y=0;y<4;y++)
		{
            for(y1=y+1;y1<4;y1++)
			{
                if(nums[x][y1]>0)
				{
                    if(nums[x][y]<=0)
					{
                        nums[x][y]=(nums[x][y1]);
                        nums[x][y1]=0;
                        y--;
                        merge=true;
                   	 }else if(nums[x][y]==nums[x][y1])
                   	 {
                        nums[x][y]=(nums[x][y]*2);
                        nums[x][y1]=0;       
                        merge=true;
                    }
                    break;
               }
            }
        }
   	}
}

//�һ�
void move_right() 
{
	merge=false;
	int x,y,y1;
    for(x=0;x<4;x++)//������
	{
        for(y=3;y>=0;y--)//����һ��
		{
            for(y1=y-1;y1>=0;y1--)
			{
                if(nums[x][y1]>0)
				{
                    if(nums[x][y]<=0)
					{
            			nums[x][y]=(nums[x][y1]);
            			nums[x][y1]=(0);
            			y++;
            			merge=true;
        			}else if(nums[x][y]==nums[x][y1])
        			{
            			nums[x][y]=(nums[x][y]*2);
            			nums[x][y1]=0; 
            			merge=true;
        			}
        			break;
                }
            }
       	}
    }
}

/*
	����:����Ƿ������Ϸ
	����ֵ:����ֵ
*/
bool check_over()
{	
	int x,y;
	for(y=0;y<4;y++)
	{	
		for (x=0;x<4;x++)
		{	
		
			if(nums[x][y]==0)
			{
				return false;
			}
			if(x>0 && nums[x][y]==nums[x-1][y])
			{
				return false;//���if����ȥ��
			}
			if(x<3 && nums[x][y]==nums[x+1][y])
			{
				return false;
			}

			if(y>0 && nums[x][y]==nums[x][y-1])
			{
				return false;//���if����ȥ��
			}
			if(y<3 && nums[x][y]==nums[x][y+1])
			{
				return false;
			}
		}
	}
	return true;  
}


