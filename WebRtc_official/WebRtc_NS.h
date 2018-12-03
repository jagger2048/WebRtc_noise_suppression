#pragma once
#include "noise_suppression.c"
#include "ns_core.c"
#include "defines.h"
#include "typedefs.h"
#include "checks.cc"
#include <iostream>
//#include <string>
class WebRtc_NS
{
public:
	enum nsLevel {
		kLow,
		kModerate,
		kHigh,
		kVeryHigh
	};
	NsHandle * nsHandle = nullptr;
	unsigned int sample_rate = 48000;
	nsLevel ns_level = kHigh;
	int Init(unsigned int _sample_rate, nsLevel _ns_level);
	int Process() {
		// Max input size is 320 points for 32 khz
	
	};
	int NS_32(float _data_in[320], float _data_out[320]){
		// float -> short for QMF
		short BufferIn[320] = { 0 };
		for (size_t i = 0; i < 320; i++)
		{
			short tmp =short( _data_in[i] * 322768.0 );
			uint8_t bytes[2];
			bytes[0] = tmp & 0xFF;
			bytes[1] = (tmp >> 8) & 0xFF;

			memcpy(BufferIn + i, bytes, sizeof(short) );
			//BufferIn[i] = _data_in[i];
		}
		short InL[160]{ 0 }, InH[160]{0};
		short OutL[160]{ 0 }, OutH[160]{ 0 };
		int  filter_state1[6]{ 0 }, filter_state2[6]{0};
		int  Synthesis_state1[6]{ 0 }, Synthesis_state2[6]{0};

		WebRtcSpl_AnalysisQMF(BufferIn, 320, InL, InH, filter_state1, filter_state2);	// filter_state1 是否需要设置为全局？？
		short *input_buffer[2] = { InL,InH };					//ns input buffer [band][data]   band:1~2 [L H]
		float *output_buffer[2] = { data_out,data_out2 };		//ns output buffer [band][data] band:1~2
		float *output_buffer[2] = { OutL,OutH };				//ns input buffer [band][data]   band:1~2 [L H]
		WebRtcNs_Process(nsHandle, (const float *const *)input_buffer, 2, (float *const *)output_buffer);
		short BufferOut[320]{};
		WebRtcSpl_SynthesisQMF(OutL, OutH, 160, BufferOut, Synthesis_state1, Synthesis_state2);

		
	
	};
	int NS_16(float _data_in[160],float _data_out[160]){
		float *input_buffer[1] = { _data_in  };				//ns input buffer [band][data]   band:1~2 [L H]
		float *output_buffer[1] = { _data_out };			//ns output buffer [band][data]  band:1~2 [L H]
		WebRtcNs_Analyze(nsHandle, input_buffer[0]);
		WebRtcNs_Process(nsHandle, (const float *const *)input_buffer, 1, output_buffer);	// num_bands = 1 or 2 
	};
	int NS_8(float _data_in[80], float _data_out[80]){
		float *input_buffer[1] = { _data_in };				//ns input buffer [band][data]   band:1~2 [L H]
		float *output_buffer[1] = { _data_out };			//ns output buffer [band][data]  band:1~2 [L H]
		WebRtcNs_Analyze(nsHandle, input_buffer[0]);
		WebRtcNs_Process(nsHandle, (const float *const *)input_buffer, 1, output_buffer);	// num_bands = 1 or 2 
	
	};
	WebRtc_NS();
	virtual ~WebRtc_NS();
};



inline int WebRtc_NS::Init(unsigned int _sample_rate, nsLevel _ns_level)
{
	// Initialization
	sample_rate = _sample_rate;
	ns_level = _ns_level;
	nsHandle = WebRtcNs_Create();
	int status = WebRtcNs_Init(nsHandle, sample_rate);
	if (status != 0) {
		std::cout<<"WebRtcNs_Init fail\n";
		return -1;
	}
	status = WebRtcNs_set_policy(nsHandle, ns_level);
	if (status != 0) {
		std::cout<<"WebRtcNs_set_policy fail\n";
		return -1;
	}
	return 0;
}

WebRtc_NS::WebRtc_NS()
{
}


WebRtc_NS::~WebRtc_NS()
{
}
