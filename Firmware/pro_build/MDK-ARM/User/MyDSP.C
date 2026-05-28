#include "MyDSP.h"
#include <math.h>
#include <string.h>

#ifndef MYDSP_DEBUG_RAW_OUTPUT
#define MYDSP_DEBUG_RAW_OUTPUT 0
#endif

//峰值幅值与对应索引
float32_t max_mag_x = 0, max_mag_y = 0, max_mag_z = 0;
uint32_t max_index_x = 0, max_index_y = 0, max_index_z = 0;
// ADXL345原始采样数据，对应ADXL345三轴加速度原始数据
static float32_t Original_input_x[LENGTH_SAMPLE];
static float32_t Original_input_y[LENGTH_SAMPLE];
static float32_t Original_input_z[LENGTH_SAMPLE];
// FIR滤波后时域信号（原始数据经过FIR滤波处理后）
static float32_t FIR_output_x[LENGTH_SAMPLE];
static float32_t FIR_output_y[LENGTH_SAMPLE];
static float32_t FIR_output_z[LENGTH_SAMPLE];
// 加汉宁窗后时域信号
static float32_t FFT_input_x[LENGTH_SAMPLE];
static float32_t FFT_input_y[LENGTH_SAMPLE];
static float32_t FFT_input_z[LENGTH_SAMPLE];
// 频谱为N/2+1点，仅需存储一半幅值，实数FFT共轭对称，无需重复存储
static float32_t FFT_output_x[LENGTH_SAMPLE];
static float32_t FFT_output_y[LENGTH_SAMPLE];
static float32_t FFT_output_z[LENGTH_SAMPLE];
// 频谱幅值数组
//static float32_t frequency;
static float32_t FFT_output_mag_x[LENGTH_SAMPLE/2+1];
static float32_t FFT_output_mag_y[LENGTH_SAMPLE/2+1];
static float32_t FFT_output_mag_z[LENGTH_SAMPLE/2+1];

/* FIR滤波系数 由fadtool工具生成 */
#define NUM_TAPS   29    // 滤波器抽头数
#define BLOCK_SIZE 32     // 每次arm_fir_f32处理块大小
uint32_t blockSize = BLOCK_SIZE;
uint32_t numBlocks = LENGTH_SAMPLE/BLOCK_SIZE;           // 需要执行FIR分块次数
// 状态缓存区，大小要求：numTaps + blockSize - 1
static float32_t firStateF32_x[BLOCK_SIZE + NUM_TAPS - 1];        
static float32_t firStateF32_y[BLOCK_SIZE + NUM_TAPS - 1];
static float32_t firStateF32_z[BLOCK_SIZE + NUM_TAPS - 1];

static void MyDSP_ComputeMagnitude(const float32_t *fftOutput, float32_t *magnitude);

// 800HZ hann窗?通滤波器 46阶
//const float32_t B[NUM_TAPS] = {
//               -0,5.094349399e-05,0.0002126742475,-0.0004989779554,-0.0009244055836,
//   0.001504608314, 0.002256840235,-0.003200700507,-0.004359227605, 0.005760520697,
//   0.007440174464,-0.009445019066, -0.01183902007,  0.01471291855,  0.01820068061,
//   -0.02250909992, -0.02797468193,  0.03518253937,  0.04524371773, -0.06054779515,
//   -0.08732147515,   0.1484210789,   0.4496337175,   0.4496337175,   0.1484210789,
//   -0.08732147515, -0.06054779515,  0.04524371773,  0.03518253937, -0.02797468193,
//   -0.02250909992,  0.01820068061,  0.01471291855, -0.01183902007,-0.009445019066,
//   0.007440174464, 0.005760520697,-0.004359227605,-0.003200700507, 0.002256840235,
//   0.001504608314,-0.0009244055836,-0.0004989779554,0.0002126742475,5.094349399e-05,
//               -0
//};

// ?通滤波器系数 fadtool生成 29阶 150HZ
//const float32_t B[NUM_TAPS] = {
//	0.0018157335f,     0.001582013792f,    -6.107207639e-18f,  -0.003683975432f,   -0.008045346476f,
//	-0.008498443291f,  -1.277260999e-17f,  0.01733288541f,     0.03401865438f,     0.0332348831f,
//	-4.021742543e-17f, -0.06737889349f,    -0.1516391635f,     -0.2220942229f,     0.7486887574f,
//	-0.2220942229f,    -0.1516391635f,     -0.06737889349f,    -4.021742543e-17f,  0.0332348831f,
//	0.03401865438f,    0.01733288541f,     -1.277260999e-17f,  -0.008498443291f,   -0.008045346476f,
//	-0.003683975432f,  -6.107207639e-18f,  0.001582013792f,    0.0018157335f
//};
// ?通滤波器系数 fadtool生成 29阶 150HZ
const float32_t B[NUM_TAPS] = {
  -0.001822523074f,  -0.001587929321f,  1.226008847e-18f,  0.003697750857f,  0.008075430058f,
  0.008530221879f,   -4.273456581e-18f, -0.01739769801f,   -0.03414586186f,  -0.03335915506f,
  8.073562366e-18f,  0.06763084233f,    0.1522061825f,     0.2229246944f,    0.2504960895f,
  0.2229246944f,     0.1522061825f,     0.06763084233f,    8.073562366e-18f, -0.03335915506f,
  -0.03414586186f,   -0.01739769801f,   -4.273456581e-18f, 0.008530221879f,  0.008075430058f,
  0.003697750857f,   1.226008847e-18f,  -0.001587929321f,  -0.001822523074f
};


void MyDSP_Init(){
    // 读取ADXL345三轴数据并转换单位
    for(int i = 0; i < LENGTH_SAMPLE; i++) {
       Original_input_x[i] = ADXL345_XTest() * 0.00390625f; 
       Original_input_y[i] = ADXL345_YTest() * 0.00390625f;
       Original_input_z[i] = ADXL345_ZTest() * 0.00390625f;
        
#if MYDSP_DEBUG_RAW_OUTPUT
        printf("%.5f,%.5f,%.5f\n", Original_input_x[i], Original_input_y[i], Original_input_z[i]);
#endif
//        // VOFA+��ʽ
//        printf("X: %.5f\n", Original_input_x[i]); // ��ӡX������
//        //printf("Y: %.5f\n", Original_input_y[i]); // ��ӡY������
      //ģ���ź�
      // Original_input_x[i] = arm_sin_f32(2*3.1415926f*50*i/1000) ;//+ arm_sin_f32(2*3.1415926f*200*i/1000);
      // printf("X: %.5f\n", Original_input_x[i]); // ��ӡX������
    }
  
    printf("DSP_Init completed.\n");
}

void arm_fir_f32_lp(){

  arm_fir_instance_f32 Sx, Sy, Sz;
  	float32_t  *inputF32, *outputF32;

	inputF32 = &Original_input_x[0];
	outputF32 = &FIR_output_x[0];

  arm_fir_init_f32(&Sx, NUM_TAPS, (float32_t *)B, &firStateF32_x[0], BLOCK_SIZE);
    arm_fir_init_f32(&Sy, NUM_TAPS, (float32_t *)B, &firStateF32_y[0], BLOCK_SIZE);
    arm_fir_init_f32(&Sz, NUM_TAPS, (float32_t *)B, &firStateF32_z[0], BLOCK_SIZE);
    
	for(int i=0; i < numBlocks; i++)
	{
		    arm_fir_f32(&Sx, inputF32 + (i * blockSize), outputF32 + (i * blockSize), blockSize);
        arm_fir_f32(&Sy, Original_input_y + (i * blockSize), FIR_output_y + (i * blockSize), blockSize);
        arm_fir_f32(&Sz, Original_input_z + (i * blockSize), FIR_output_z + (i * blockSize), blockSize);
        //printf("FIR Output X: %.2f, Y: %.2f, Z: %.2f\n", FIR_output_x[i], FIR_output_y[i], FIR_output_z[i]);
    }
  
#if MYDSP_DEBUG_RAW_OUTPUT
	for(int i=0; i<LENGTH_SAMPLE; i++)	printf("%.5f,%.5f,%.5f\n", FIR_output_x[i],FIR_output_y[i],FIR_output_z[i]);//VOFA+��ʽ,��ӡX���˲��������
#endif
    printf("FIR filter processing completed.\n");
}
/*@par Parameters of the window
  
  | Parameter                             | Value              |
  | ------------------------------------: | -----------------: |
  | Peak sidelobe level                   |           31.5 dB  |
  | Normalized equivalent noise bandwidth |          1.5 bins  |
  | 3 dB bandwidth                        |       1.4382 bins  |
  | Flatness                              |        -1.4236 dB  |
  | Recommended overlap                   |              50 %  |

 */
 float32_t hanning_window[LENGTH_SAMPLE];// 汉宁窗系数数组
void arm_hanning_f32(float32_t * pDst,uint32_t blockSize)
{
   float32_t k = 2.0f / ((float32_t) blockSize);
   float32_t w;
    // 计算汉宁窗系数
   for(uint32_t i=0;i<blockSize;i++)
   {
     w = PI * i * k;
     w = 0.5f * (1.0f - cosf (w));
     pDst[i] = w;
   }
   //分别对 XYZ 三轴信号加窗
   for(int i = 0; i < LENGTH_SAMPLE; i++) FFT_input_x[i] = FIR_output_x[i] * hanning_window[i];
   for(int i = 0; i < LENGTH_SAMPLE; i++) FFT_input_y[i] = FIR_output_y[i] * hanning_window[i];
   for(int i = 0; i < LENGTH_SAMPLE; i++) FFT_input_z[i] = FIR_output_z[i] * hanning_window[i];
   
#if MYDSP_DEBUG_RAW_OUTPUT
   for(int i=0; i<LENGTH_SAMPLE; i++)	printf("%.5f,%.5f,%.5f\n", FFT_input_x[i],FFT_input_y[i],FFT_input_z[i]);//VOFA+��ʽ,��ӡX���˲��������
#endif

   printf("Hanning window processing completed.\n");
}

//FFT����
void MyFFT_Process()
{
  
    arm_rfft_fast_instance_f32 S;

    arm_rfft_fast_init_f32(&S, LENGTH_SAMPLE);
    
    arm_rfft_fast_f32(&S, FFT_input_x, FFT_output_x, 0); // X��FFT
    arm_rfft_fast_f32(&S, FFT_input_y, FFT_output_y, 0); // Y��FFT
    arm_rfft_fast_f32(&S, FFT_input_z, FFT_output_z, 0); // Z��FFT
  //arm_cfft_f32(&S , fft_buf_float , 0 , 1);  
  
    MyDSP_ComputeMagnitude(FFT_output_x, FFT_output_mag_x);
    MyDSP_ComputeMagnitude(FFT_output_y, FFT_output_mag_y);
    MyDSP_ComputeMagnitude(FFT_output_z, FFT_output_mag_z);
    

  //   for(int i = 0; i < FFT_POINTS; i++) 
  //       printf("FFT Output X[%d]: %.6f\n", i, FFT_output_mag_x[i]); // VOFA+��ʽ,��ӡX��FFT����ķ���
  
}

static void MyDSP_ComputeMagnitude(const float32_t *fftOutput, float32_t *magnitude)
{
    if (fftOutput == NULL || magnitude == NULL) return;

    magnitude[0] = (fftOutput[0] >= 0.0f) ? fftOutput[0] : -fftOutput[0];

    for(uint32_t i = 1; i < LENGTH_SAMPLE / 2; i++) {
        float32_t real = fftOutput[2 * i];
        float32_t imag = fftOutput[2 * i + 1];
        magnitude[i] = sqrtf(real * real + imag * imag);
    }

    magnitude[LENGTH_SAMPLE / 2] = (fftOutput[1] >= 0.0f) ? fftOutput[1] : -fftOutput[1];
}

//��DSP��������
void MyDSP_Process()
{
    // ��ʼ��_����ԭʼ�ź�
    MyDSP_Init();
    
    // FIR��ͨ�˲�����
    arm_fir_f32_lp();
    
    // ���ɺ��������������мӴ�����
    arm_hanning_f32(hanning_window, LENGTH_SAMPLE);
    // FFT����
    MyFFT_Process();
  
    //������ѧ���ֵ�����ҳ������ȺͶ�Ӧ��Ƶ��
    arm_max_f32(FFT_output_mag_x, FFT_POINTS, &max_mag_x, &max_index_x);
    arm_max_f32(FFT_output_mag_y, FFT_POINTS, &max_mag_y, &max_index_y);
    arm_max_f32(FFT_output_mag_z, FFT_POINTS, &max_mag_z, &max_index_z);
    //��ӡ�����ȺͶ�Ӧ��Ƶ��
    // ��ӡ�����ȺͶ�ӦƵ��
    printf("Max Magnitude X: %.6f at Frequency: %.2f Hz\n", max_mag_x, (float32_t)(max_index_x * FREQUENCY_STEP));
    printf("Max Magnitude Y: %.6f at Frequency: %.2f Hz\n", max_mag_y, (float32_t)(max_index_y * FREQUENCY_STEP));
    printf("Max Magnitude Z: %.6f at Frequency: %.2f Hz\n", max_mag_z, (float32_t)(max_index_z * FREQUENCY_STEP));

    // 高级频谱分析（X轴）
    SpectrumAnalysis analysis_x;
    MyDSP_Analyze_Spectrum(&analysis_x, 0); // 分析X轴

    // 显示X轴详细分析结果
    MyDSP_Display_Analysis(&analysis_x, 0);

    // 同时在串口打印详细分析结果
    printf("\n=== Advanced Spectrum Analysis (X Axis) ===\n");
    printf("Total Energy: %.6e\n", analysis_x.total_energy);
    printf("Spectral Centroid: %.2f Hz\n", analysis_x.centroid_frequency);
    printf("Mean Magnitude: %.6f\n", analysis_x.mean_magnitude);
    printf("Variance Magnitude: %.6e\n", analysis_x.variance_magnitude);
    printf("Peak-to-Average Ratio: %.2f\n", analysis_x.peak_to_average_ratio);

    if(analysis_x.num_peaks > 0) {
        printf("Detected Peaks (%u):\n", analysis_x.num_peaks);
        for(uint32_t i = 0; i < analysis_x.num_peaks; i++) {
            printf("  Peak %u: %.2f Hz, Magnitude: %.6f\n",
                   i+1, analysis_x.peaks[i].frequency, analysis_x.peaks[i].magnitude);
        }
    }

    printf("Energy Distribution (%%):\n");
    for(uint8_t band = 0; band < FREQ_BANDS; band++) {
        printf("  Band %u: %.1f%%\n", band+1, analysis_x.energy_distribution[band]);
    }
    printf("==========================================\n\n");
}

void test()
{
//  //测试arm_max_f32最大值查找函数
//  float32_t test_data[] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
//  float32_t max_value;
//  uint32_t max_index;
//  arm_max_f32(test_data, 5, &max_value, &max_index);
//  //在OLED上显示测试结果
//  OLED_Printf(0, 0, OLED_6X8, "Max Value: ");
//  OLED_ShowFloatNum(70, 0, max_value, 1, 6, OLED_6X8); // 显示最大值
//  OLED_Printf(0, 8, OLED_6X8, "Max Index: ");
//  OLED_ShowNum(70, 8, max_index, 1, OLED_6X8);// 显示最大值索引
//  OLED_Update();

  //测试arm_cmplx_mag_f32复数求模函数
  float32_t complex_data[] = {1.0f, 2.0f, 3.0f, 4.0f}; // 实部、虚部交替存储
  float32_t magnitude[2]; // 存储幅值结果
  arm_cmplx_mag_f32(complex_data, magnitude, 2); // 计算模值
  // 在OLED上显示测试结果
  OLED_Printf(0, 0, OLED_6X8, "Magnitude: ");
  OLED_ShowFloatNum(70, 0, magnitude[0], 1, 6, OLED_6X8); // 显示第一个模值
  OLED_ShowFloatNum(70, 8, magnitude[1], 1, 6, OLED_6X8); // 显示第ER个模值
  OLED_Update();
}
/*
*********************************************************************************************************
*	函 数 名: arm_rfft_fast_f32_app
*	功能说明: 使用函数arm_rfft_fast_f32进行1024点实数FFT的应用示例，与使用函数arm_cfft_f32复数FFT效果对比。
*********************************************************************************************************
*/
/* 测试数组长度 */
#define TEST_LENGTH_SAMPLES 2048 
static float32_t testInput_f32_10khz[TEST_LENGTH_SAMPLES];
static float32_t testOutput_f32_10khz[TEST_LENGTH_SAMPLES];
static float32_t testOutput[TEST_LENGTH_SAMPLES];

/* FFT参数 */
uint32_t fftSize = 1024; 
uint32_t ifftFlag = 0; 
uint32_t doBitReverse = 1; 
void arm_rfft_fast_f32_app(void)
{
	uint16_t i;
	arm_rfft_fast_instance_f32 S;
	
	/* 实数FFT长度设置 */
	fftSize = 1024; 
	/* 正变换 */
  ifftFlag = 0; 
	
	/* 初始化结构体S中的参数 */
 	arm_rfft_fast_init_f32(&S, fftSize);
	
	/* 实数信号，实部、虚部交替存储 */
	for(i=0; i<1024; i++)
	{
		/* 50Hz正弦波信号，采样率1KHz */
		testInput_f32_10khz[i] = 1.2f*arm_sin_f32(2*3.1415926f*50*i/1000)+1;
	}
	
	/* 1024点实数快速FFT运算 */ 
	arm_rfft_fast_f32(&S, testInput_f32_10khz, testOutput_f32_10khz, ifftFlag);
	
	/* 为了方便和arm_cfft_f32复数FFT结果对比，计算1024点模值
	   实际arm_rfft_fast_f32只需要512点  
	*/ 
 	arm_cmplx_mag_f32(testOutput_f32_10khz, testOutput, fftSize);

	/* 串口打印幅值结果 */
	for(i=0; i<fftSize; i++)
	{
		printf("%f\r\n", testOutput[i]);
	}

	printf("****************************分隔线***************************************\r\n");
// 构造纯实数输入，使用标准复数FFT
	for(i=0; i<1024; i++)
	{
		/* 虚部全部置0 */
		testInput_f32_10khz[i*2+1] = 0;
		
		/* 50Hz正弦波信号，采样率1KHz，作为实部 */
		testInput_f32_10khz[i*2] = 1.2f*arm_sin_f32(2*3.1415926f*50*i/1000)+1;
	}
	
	arm_cfft_f32(&arm_cfft_sR_f32_len1024, testInput_f32_10khz, ifftFlag, doBitReverse);
	
	/* 计算模值 */ 
 	arm_cmplx_mag_f32(testInput_f32_10khz, testOutput, fftSize);

	/* 串口打印幅值结果 */
	for(i=0; i<fftSize; i++)
	{
		printf("%f\r\n", testOutput[i]);
	}

}

// ============================================
// 高级频谱分析函数实现
// ============================================

/**
 * @brief 多峰值检测算法
 * @param spectrum 输入频谱（幅度谱）
 * @param size 频谱大小
 * @param peaks 输出峰值数组
 * @param max_peaks 最大检测峰值数量
 * @param min_distance 峰值最小频率间隔（索引距离）
 */
void MyDSP_Find_Peaks(float32_t* spectrum, uint32_t size,
                      SpectralComponent* peaks, uint32_t max_peaks,
                      uint32_t min_distance) {
    uint32_t i, j;
    uint32_t peak_count = 0;

    if(spectrum == NULL || peaks == NULL || size < 3 || max_peaks == 0) return;
    if(size > FFT_POINTS) size = FFT_POINTS;

    for(i = 0; i < max_peaks; i++) {
        peaks[i].frequency = 0.0f;
        peaks[i].magnitude = 0.0f;
        peaks[i].index = 0;
    }

    // 临时数组标记是否为峰值
    static uint8_t is_peak[FFT_POINTS];
    memset(is_peak, 0, sizeof(is_peak));

    // 第一步：检测所有局部最大值
    // 忽略边界点（i=0和i=size-1）
    for(i = 1; i < size - 1; i++) {
        if(spectrum[i] > spectrum[i-1] && spectrum[i] > spectrum[i+1]) {
            is_peak[i] = 1;
        }
    }

    // 第二步：按幅度排序并选择前max_peaks个峰值
    // 简单实现：多次查找最大值，每次找到后将其周围区域置零
    for(j = 0; j < max_peaks; j++) {
        float32_t max_val = 0.0f;
        uint32_t max_idx = 0;

        // 查找当前最大峰值
        for(i = 0; i < size; i++) {
            if(is_peak[i] && spectrum[i] > max_val) {
                max_val = spectrum[i];
                max_idx = i;
            }
        }

        // 如果没有找到更多峰值，退出循环
        if(max_val == 0.0f) {
            break;
        }

        // 保存峰值信息
        peaks[peak_count].index = max_idx;
        peaks[peak_count].frequency = max_idx * FREQUENCY_STEP;
        peaks[peak_count].magnitude = max_val;
        peak_count++;

        // 将当前峰值周围区域置零，避免重复检测
        uint32_t start = (max_idx > min_distance) ? max_idx - min_distance : 0;
        uint32_t end = (max_idx + min_distance < size) ? max_idx + min_distance : size - 1;

        for(i = start; i <= end; i++) {
            is_peak[i] = 0;
        }
    }

    // 第三步：按频率排序（升序）
    for(i = 0; i < peak_count; i++) {
        for(j = i + 1; j < peak_count; j++) {
            if(peaks[i].frequency > peaks[j].frequency) {
                SpectralComponent temp = peaks[i];
                peaks[i] = peaks[j];
                peaks[j] = temp;
            }
        }
    }
}

/**
 * @brief 计算频谱能量分布
 * @param spectrum 输入频谱（幅度谱）
 * @param size 频谱大小
 * @param distribution 输出能量分布数组
 * @param bands 频段数量
 */
void MyDSP_Compute_Energy_Distribution(float32_t* spectrum, uint32_t size,
                                       float32_t* distribution, uint32_t bands) {
    uint32_t i, band;
    float32_t total_energy = 0.0f;

    if(spectrum == NULL || distribution == NULL || size == 0 || bands == 0) return;

    // 计算总能量
    for(i = 0; i < size; i++) {
        total_energy += spectrum[i] * spectrum[i];
    }

    // 清空分布数组
    for(band = 0; band < bands; band++) {
        distribution[band] = 0.0f;
    }

    // 计算每个频段的能量
    uint32_t band_size = size / bands;
    for(band = 0; band < bands; band++) {
        uint32_t start_idx = band * band_size;
        uint32_t end_idx = (band == bands - 1) ? size - 1 : (band + 1) * band_size - 1;

        for(i = start_idx; i <= end_idx; i++) {
            distribution[band] += spectrum[i] * spectrum[i];
        }

        // 归一化：计算每个频段能量占总能量的百分比
        if(total_energy > 0.0f) {
            distribution[band] = distribution[band] / total_energy * 100.0f;
        }
    }
}

/**
 * @brief 计算频谱质心
 * @param spectrum 输入频谱（幅度谱）
 * @param size 频谱大小
 * @return 频谱质心（频率）
 */
float32_t MyDSP_Compute_Spectral_Centroid(float32_t* spectrum, uint32_t size) {
    uint32_t i;
    float32_t weighted_sum = 0.0f;
    float32_t energy_sum = 0.0f;

    if(spectrum == NULL || size == 0) return 0.0f;

    for(i = 0; i < size; i++) {
        float32_t frequency = i * FREQUENCY_STEP;
        float32_t energy = spectrum[i] * spectrum[i];

        weighted_sum += frequency * energy;
        energy_sum += energy;
    }

    if(energy_sum > 0.0f) {
        return weighted_sum / energy_sum;
    } else {
        return 0.0f;
    }
}

/**
 * @brief 计算平均幅度
 * @param spectrum 输入频谱（幅度谱）
 * @param size 频谱大小
 * @return 平均幅度
 */
float32_t MyDSP_Compute_Mean_Magnitude(float32_t* spectrum, uint32_t size) {
    uint32_t i;
    float32_t sum = 0.0f;

    if(spectrum == NULL || size == 0) return 0.0f;

    for(i = 0; i < size; i++) {
        sum += spectrum[i];
    }

    if(size > 0) {
        return sum / size;
    } else {
        return 0.0f;
    }
}

/**
 * @brief 计算幅度方差
 * @param spectrum 输入频谱（幅度谱）
 * @param size 频谱大小
 * @param mean 平均幅度
 * @return 幅度方差
 */
float32_t MyDSP_Compute_Variance_Magnitude(float32_t* spectrum, uint32_t size,
                                          float32_t mean) {
    uint32_t i;
    float32_t sum_squared_diff = 0.0f;

    if(spectrum == NULL || size == 0) return 0.0f;

    for(i = 0; i < size; i++) {
        float32_t diff = spectrum[i] - mean;
        sum_squared_diff += diff * diff;
    }

    if(size > 0) {
        return sum_squared_diff / size;
    } else {
        return 0.0f;
    }
}

/**
 * @brief 计算峰均比
 * @param spectrum 输入频谱（幅度谱）
 * @param size 频谱大小
 * @param max_magnitude 最大幅度值
 * @param mean 平均幅度
 * @return 峰均比
 */
float32_t MyDSP_Compute_Peak_to_Average_Ratio(float32_t* spectrum, uint32_t size,
                                             float32_t max_magnitude, float32_t mean) {
    (void)spectrum;
    (void)size;

    if(mean > 0.0f) {
        return max_magnitude / mean;
    } else {
        return 0.0f;
    }
}

/**
 * @brief 执行完整的频谱分析
 * @param analysis 输出分析结果结构体
 * @param axis 轴选择：0=X, 1=Y, 2=Z
 */
void MyDSP_Analyze_Spectrum(SpectrumAnalysis* analysis, uint8_t axis) {
    float32_t* spectrum = NULL;
    uint32_t size = FFT_POINTS;

    if(analysis == NULL) return;
    memset(analysis, 0, sizeof(SpectrumAnalysis));

    // 根据轴选择频谱数据
    switch(axis) {
        case 0: // X轴
            spectrum = FFT_output_mag_x;
            break;
        case 1: // Y轴
            spectrum = FFT_output_mag_y;
            break;
        case 2: // Z轴
            spectrum = FFT_output_mag_z;
            break;
        default:
            spectrum = FFT_output_mag_x;
            break;
    }

    if(spectrum == NULL) {
        return;
    }

    // 1. 峰值检测
    MyDSP_Find_Peaks(spectrum, size, analysis->peaks, MAX_PEAKS, MIN_PEAK_DISTANCE);
    analysis->num_peaks = 0;

    // 计算实际检测到的峰值数量
    for(uint32_t i = 0; i < MAX_PEAKS; i++) {
        if(analysis->peaks[i].magnitude > 0.0f) {
            analysis->num_peaks = i + 1;
        } else {
            break;
        }
    }

    // 2. 计算能量分布
    MyDSP_Compute_Energy_Distribution(spectrum, size, analysis->energy_distribution, FREQ_BANDS);

    // 3. 计算频谱质心
    analysis->centroid_frequency = MyDSP_Compute_Spectral_Centroid(spectrum, size);

    // 4. 计算平均幅度
    analysis->mean_magnitude = MyDSP_Compute_Mean_Magnitude(spectrum, size);

    // 5. 计算幅度方差
    analysis->variance_magnitude = MyDSP_Compute_Variance_Magnitude(spectrum, size, analysis->mean_magnitude);

    // 6. 计算总能量
    analysis->total_energy = 0.0f;
    for(uint32_t i = 0; i < size; i++) {
        analysis->total_energy += spectrum[i] * spectrum[i];
    }

    // 7. 计算峰均比
    float32_t max_magnitude = 0.0f;
    for(uint32_t i = 0; i < size; i++) {
        if(spectrum[i] > max_magnitude) {
            max_magnitude = spectrum[i];
        }
    }
    analysis->peak_to_average_ratio = MyDSP_Compute_Peak_to_Average_Ratio(spectrum, size, max_magnitude, analysis->mean_magnitude);
}

/**
 * @brief 在OLED上显示频谱分析结果
 * @param analysis 频谱分析结果
 * @param axis 轴选择：0=X, 1=Y, 2=Z
 */
void MyDSP_Display_Analysis(SpectrumAnalysis* analysis, uint8_t axis) {
    char buffer[64];
    uint8_t y_offset = 0;

    if(analysis == NULL) return;

    // 清屏
    OLED_Clear();

    // 显示标题
    const char* axis_names[] = {"X", "Y", "Z"};
    sprintf(buffer, "Axis %s Analysis", axis_names[axis % 3]);
    OLED_Printf(0, y_offset, OLED_6X8, buffer);
    y_offset += 8;

    // 显示峰值信息
    if(analysis->num_peaks > 0) {
        OLED_Printf(0, y_offset, OLED_6X8, "Top Peaks:");
        y_offset += 8;

        for(uint32_t i = 0; i < analysis->num_peaks && i < 3; i++) { // 最多显示3个峰值
            sprintf(buffer, "%lu: %.1fHz %.3f",
                   i+1,
                   analysis->peaks[i].frequency,
                   analysis->peaks[i].magnitude);
            OLED_Printf(0, y_offset, OLED_6X8, buffer);
            y_offset += 8;
        }
    } else {
        OLED_Printf(0, y_offset, OLED_6X8, "No peaks detected");
        y_offset += 8;
    }

    // 显示频谱质心
    sprintf(buffer, "Centroid: %.1f Hz", analysis->centroid_frequency);
    OLED_Printf(0, y_offset, OLED_6X8, buffer);
    y_offset += 8;

    // 显示总能量
    sprintf(buffer, "Energy: %.3e", analysis->total_energy);
    OLED_Printf(0, y_offset, OLED_6X8, buffer);
    y_offset += 8;

    // 显示峰均比
    sprintf(buffer, "PAR: %.2f", analysis->peak_to_average_ratio);
    OLED_Printf(0, y_offset, OLED_6X8, buffer);
    y_offset += 8;

    // 显示能量分布（如果空间允许）
    if(y_offset + 16 < 64) { // 确保不会超出屏幕
        OLED_Printf(0, y_offset, OLED_6X8, "Energy Dist(%):");
        y_offset += 8;

        for(uint8_t band = 0; band < FREQ_BANDS && band < 4; band++) { // 最多显示4个频段
            if(band % 2 == 0) { // 左半屏
                sprintf(buffer, "B%d:%.1f", band+1, analysis->energy_distribution[band]);
                OLED_Printf(0, y_offset, OLED_6X8, buffer);
            } else { // 右半屏
                sprintf(buffer, "B%d:%.1f", band+1, analysis->energy_distribution[band]);
                OLED_Printf(64, y_offset, OLED_6X8, buffer);
                y_offset += 8;
            }
        }
    }

    OLED_Update();
}
