#ifndef MYDSP_H_
#define MYDSP_H_

#include "main.h"
#include "stdlib.h"
#include "arm_math.h"
#include "arm_const_structs.h"

#include "ADXL345.h"
#include "OLED.h"
#include "stdio.h"


//ADXL345的采样率为3200HZ,即为每秒采集3200个数据
//本项目使用1024点FFT变换,即每次采集1024个数据进行频谱分析
//采集时间1024/3200秒数据,约为0.32秒
#define LENGTH_SAMPLE 1024    //采样点数,必须为2的整数次幂,常用值1024、2048
#define RATE_SAMPLE    3200   //采样率,单位为Hz
#define FFT_POINTS     (LENGTH_SAMPLE/2+1)  //FFT变换后的有效点数 N/2+1
#define FREQUENCY_STEP ((float32_t)RATE_SAMPLE / (float32_t)LENGTH_SAMPLE) //频率步长,单位为Hz

// 高级频谱分析配置
#define MAX_PEAKS 10          // 最大检测峰值数量
#define FREQ_BANDS 5          // 频段数量（用于能量分布分析）
#define MIN_PEAK_DISTANCE 5   // 峰值最小频率间隔（单位：频率索引）

extern float32_t hanning_window[];

// 频谱分析数据结构
typedef struct {
    float32_t frequency;      // 频率(Hz)
    float32_t magnitude;      // 幅度
    uint32_t index;          // 频率索引
} SpectralComponent;

typedef struct {
    SpectralComponent peaks[MAX_PEAKS];  // 主要峰值
    uint32_t num_peaks;                  // 检测到的峰值数量
    float32_t total_energy;              // 总能量
    float32_t energy_distribution[FREQ_BANDS]; // 频段能量分布
    float32_t centroid_frequency;        // 频谱质心
    float32_t mean_magnitude;           // 平均幅度
    float32_t variance_magnitude;       // 幅度方差
    float32_t peak_to_average_ratio;    // 峰均比
} SpectrumAnalysis;


void MyDSP_Init(void);
void arm_fir_f32_lp(void);
void arm_hanning_f32(float32_t * pDst,uint32_t blockSize);
void MyFFT_Process(void);
void MyDSP_Process(void);
//void test(void);
void arm_rfft_fast_f32_app(void);

// 高级频谱分析函数
void MyDSP_Analyze_Spectrum(SpectrumAnalysis* analysis, uint8_t axis); // axis: 0=X, 1=Y, 2=Z
void MyDSP_Find_Peaks(float32_t* spectrum, uint32_t size, SpectralComponent* peaks, uint32_t max_peaks, uint32_t min_distance);
void MyDSP_Compute_Energy_Distribution(float32_t* spectrum, uint32_t size, float32_t* distribution, uint32_t bands);
float32_t MyDSP_Compute_Spectral_Centroid(float32_t* spectrum, uint32_t size);
float32_t MyDSP_Compute_Mean_Magnitude(float32_t* spectrum, uint32_t size);
float32_t MyDSP_Compute_Variance_Magnitude(float32_t* spectrum, uint32_t size, float32_t mean);
float32_t MyDSP_Compute_Peak_to_Average_Ratio(float32_t* spectrum, uint32_t size, float32_t max_magnitude, float32_t mean);

// 显示函数
void MyDSP_Display_Analysis(SpectrumAnalysis* analysis, uint8_t axis);

#endif /* MYDSP_H_ */
