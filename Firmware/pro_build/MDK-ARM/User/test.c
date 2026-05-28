//测试程序
//按键
//    OLED_ShowNum(64,16, HAL_GPIO_ReadPin(KEY_UP_GPIO_Port, KEY_UP_Pin), 1, OLED_8X16);OLED_Update();
//    OLED_ShowNum(64,48, HAL_GPIO_ReadPin(KEY_DOWN_GPIO_Port, KEY_DOWN_Pin), 1, OLED_8X16);OLED_Update();
//    OLED_ShowNum(40,32, HAL_GPIO_ReadPin(KEY_ENTER_GPIO_Port, KEY_ENTER_Pin), 1, OLED_8X16);OLED_Update();
//    OLED_ShowNum(88,32, HAL_GPIO_ReadPin(KEY_BACK_GPIO_Port, KEY_BACK_Pin), 1, OLED_8X16);OLED_Update();

//开场动画，由于有点慢，所以放到这里，解生调试时间
//  OLED_ShowStartImage(32,0,64,64,LOGO_ST);//OLED_ShowStartImage这个是特意修改的动画寒素，复制的前面的逻辑
//  HAL_Delay(150);//挺会
//  OLED_ReverseArea(32, 0,64,64);OLED_Update();/*反色*/
//  OLED_ShowStartString(0,64-8,"press any key to enter.",OLED_6X8);
//  
//  HAL_Delay(1000);
//  OLED_Clear();				//清平
//  OLED_Update();				//更新

//2025年6月3日
//原来DSP  c文件中的所有代码
/*
#include "MyDSP.h"

// 定义滤波器结构体（保存状态）
typedef struct {
    float alpha;    // 滤波系数
    float prev_out; // 上一次输出
} LPF_1st;

// 初始化滤波器
void LPF_Init(LPF_1st* filter, float fc, float sample_rate) {
    float dt = 1.0f / sample_rate;
    filter->alpha = (2 * PI * fc * dt) / (1 + 2 * PI * fc * dt);
    filter->prev_out = 0.0f; // 初始值设为0或第一次采样值
}

// 执行滤波
float LPF_Update(LPF_1st* filter, float input) {
    filter->prev_out = filter->alpha * input + (1 - filter->alpha) * filter->prev_out;
    return filter->prev_out;
}
//typedef int32_t q31_t;
//#define Q 21 // Q1.31格式，保留21位小数
//q31_t prev_out_q;

//q31_t LPF_Update_Q31(q31_t input, q31_t alpha_q) {
//    prev_out_q = alpha_q*input + ( (1<<Q) - alpha_q )*prev_out_q;
//    return prev_out_q >> Q;
//}

 //我现在已经设计好了ADXL345的驱动程序，现在我要设计一个MyDSP的库，用来处理ADXL345的数据
//这个库的功能是：XY轴的毛刺处理（全频带小噪声），Z轴低频抖动处理（0Hz附近能量集中）
//已知的是，ADXL345由SPI驱动，输出速率3200HZ,同时我的工程配备了CMSIS-DSP库
LPF_1st lpf_x, lpf_y, lpf_z;//滤波器实例化
void MyDSP_Init()
{
    LPF_Init(&lpf_x, 10.0f, 3200.0f);//初始化
    LPF_Init(&lpf_y, 10.0f, 3200.0f);
    LPF_Init(&lpf_z, 5.1f, 3200.0f);    
}
void MyDSP_Process(void)
{
    //①读取ADXL345的数据②转换为g单位（全分辨率模式，3.90625mg/LSB）③滤波处理
    // 输出处理后的数据_VOFA
    printf("%.5f,%.5f,%.5f\r\n",LPF_Update(&lpf_x, ADXL345_XTest() * 0.00390625f) ,
                                  LPF_Update(&lpf_y, ADXL345_YTest() * 0.00390625f),
                                  LPF_Update(&lpf_z, ADXL345_ZTest() * 0.00390625f)
                                  );
}
//信号预处理中，我们现在已经实现了采集和滤波去噪，接下来我们要通过FFT进行频域分析从而实现特征提取
//通过特征提取，我们可以实现电机故障诊断，判断电机是否处于正常工作状态
//   ***********输入与输出缓冲************************ 
#define LENGTH_SAMPLE 1024
static float32_t fft_input_x[LENGTH_SAMPLE];
static float32_t fft_input_y[LENGTH_SAMPLE];
static float32_t fft_input_z[LENGTH_SAMPLE];
//长度为N/2+1，因为FFT的输出是对称的，只需要保留一半，而加一个点，是为了保留直流分量
static float32_t fft_output_x[LENGTH_SAMPLE/2+1];
static float32_t fft_output_y[LENGTH_SAMPLE/2+1];
static float32_t fft_output_z[LENGTH_SAMPLE/2+1];
//频谱的幅度和相位以及频率
static float32_t frequency;
static float32_t fft_output_mag_x[LENGTH_SAMPLE/2+1];
static float32_t fft_output_mag_y[LENGTH_SAMPLE/2+1];
static float32_t fft_output_mag_z[LENGTH_SAMPLE/2+1];
//static float32_t fft_output_phase_x[LENGTH_SAMPLE/2+1];
//static float32_t fft_output_phase_y[LENGTH_SAMPLE/2+1];
//static float32_t fft_output_phase_z[LENGTH_SAMPLE/2+1];
//  ***********FFT处理函数************************ 
void MyFFT_PhaseRadians(float32_t *_ptr,float32_t *_phase,uint16_t _FFTPoints,float32_t _CmpValue)
{
    float32_t a,j,phase,magnitude;

    for(int i=0;i<_FFTPoints;i++)
    {
        a= _ptr[2*i];//实部
        j= _ptr[2*i+1];//虚部

        magnitude = sqrt(a*a+j*j);//求幅度,即模,即绝对值,即振幅,aquare root(a^2+b^2)
        phase = atan2(j,a);//求相位,atan2(b,a),返回的是-PI到PI之间的值(-PI,PI]

        if(_CmpValue>magnitude)
            _phase[i]=0.0f;//幅度小于阈值,相位为0
        else
            _phase[i]=phase*180.0f/3.1415926F;//幅度大于阈值,相位由弧度转换为角度
    }

}
void MyFFT_Process()
{
    // 定义FFT变换的结构体
    arm_rfft_fast_instance_f32 S;
    // 初始化FFT变换
    arm_rfft_fast_init_f32(&S, LENGTH_SAMPLE);
    //采集数据，作为FFT输入信号
    for(int i=0;i<LENGTH_SAMPLE;i++)
    {
        fft_input_x[i] = ADXL345_XTest();
        fft_input_y[i] = ADXL345_YTest();
        fft_input_z[i] = ADXL345_ZTest();
    }
    //1024点FFT变换
    arm_rfft_fast_f32(&S, fft_input_x, fft_output_x, 0);//0代表FFT正变换    
    arm_rfft_fast_f32(&S, fft_input_y, fft_output_y, 0);//0代表FFT正变换
    arm_rfft_fast_f32(&S, fft_input_z, fft_output_z, 0);//0代表FFT正变换
    //求FFT的模值(未归一化)
    arm_cmplx_mag_f32(fft_output_x, fft_output_mag_x, LENGTH_SAMPLE/2+1);
    arm_cmplx_mag_f32(fft_output_y, fft_output_mag_y, LENGTH_SAMPLE/2+1);
    arm_cmplx_mag_f32(fft_output_z, fft_output_mag_z, LENGTH_SAMPLE/2+1);
    //求FFT的频率点
    frequency = 3200.0f/2.0f/(LENGTH_SAMPLE/2+1);//采样频率为3200HZ，频率点为N/2+1,除以2是因为对称
    //求FFT的相位点
    //MyFFT_PhaseRadians(fft_output_x, fft_output_phase_x, LENGTH_SAMPLE/2+1, 0.1f);//0.1是幅度阈值
    //输出FFT的结果（幅值和对应的频点）
    for(int i=0;i<LENGTH_SAMPLE/2+1;i++)
    {
        frequency = 3200.0f/2.0f/(LENGTH_SAMPLE/2+1);//采样频率为3200HZ，频率点为N/2+1,除以2是因为对称
        printf("%.5f,%.5f\r\n", fft_output_mag_z[i],i*frequency );// 
    }

    //同理，输出Y轴和Z轴的FFT结果
//    arm_cmplx_mag_f32(fft_output_y, fft_output_mag_y, LENGTH_SAMPLE/2+1);MyFFT_PhaseRadians(fft_output_y, fft_output_phase_y, LENGTH_SAMPLE/2+1, 0.1f);
//    for(int i=0;i<LENGTH_SAMPLE/2+1;i++)    printf("%.5f,%.5f\r\n", fft_output_mag_y[i], fft_output_phase_y[i]);
//    arm_cmplx_mag_f32(fft_output_z, fft_output_mag_z, LENGTH_SAMPLE/2+1);MyFFT_PhaseRadians(fft_output_z, fft_output_phase_z, LENGTH_SAMPLE/2+1, 0.1f);
//    for(int i=0;i<LENGTH_SAMPLE/2+1;i++)    printf("%.5f,%.5f\r\n", fft_output_mag_z[i], fft_output_phase_z[i]);
    
}

*/