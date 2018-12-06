#pragma once
#include "noise_suppression.h"
#include "ns_core.h"
#include <iostream>


#include "signal_processing_library.h"
#include "splitting_filter.h"
#include "three_band_filter_bank.h"
#include "audio_util.h"

/*
	WebRtc noise suppression module had been supported 48 khz sample reate since c5ebbdq branch.
	The noise suppression process flow can be summed up as below:

	audio data in	->	if fs > 16khz	->	split into frequency bands (16kdz per band,numMaxUpperBands is 2,that is 48 khz)
										->	analyze / process per bands
										->	merge frequency bands
										->	output
					-> if fs < 16khz	->	analyze / process per bands
										->	output
	WebRtc uses AudioBuffer class to split audio date into different bands.								

*/
//#ifndef nullptr
//#define nullptr 0
//#endif

class WebRtc_NS
{
public:
	enum nsLevel {
		kLow,
		kModerate,
		kHigh,
		kVeryHigh
	};
	unsigned int sample_rate_ = 48000;
	nsLevel ns_level_ = kHigh;

	int Init(unsigned int sample_rate, nsLevel _ns_level);
	void Process(float *_data_in, float *_data_out, size_t data_size);
	void frame_process(float *_data_in, float *_data_out);;

	WebRtc_NS();
	virtual ~WebRtc_NS();
private:
	NsHandle * nsHandle = 0;
	size_t num_per_frame_ = 160;
	webrtc::TwoBandsStates TwoBands;
	webrtc::ThreeBandFilterBank *three_bands_filter_48k = 0;
};



inline int WebRtc_NS::Init(unsigned int sample_rate, nsLevel ns_level)
{
	// Initialization
	if (sample_rate == 8000 || sample_rate == 16000 || sample_rate == 32000 || sample_rate == 48000 || sample_rate == 44100)
	{
		if (sample_rate == 44100) {
			std::cout << "Warnning : 44.1 khz sample rate is not support originally by webrtc,processing blow regard it as 48 khz for a test.\n";
			// It's a good choice to UpResample the sample rate to 48k.
			// If not, maybe you can modify the ThreeBandFilterBank class's intialization
			// to support 44100 althought it is constructed for 48khz .

			sample_rate_ = 48000;
		}
		else {
			sample_rate_ = sample_rate;
		}

	}
	else {
		std::cout << "Cann't support this sample rate ->" << sample_rate << std::endl;
		return -1;
	}
	num_per_frame_ = sample_rate_ / 100;
	ns_level_ = ns_level;
	nsHandle = WebRtcNs_Create();
	int status = WebRtcNs_Init(nsHandle, sample_rate_);
	if (status != 0) {
		std::cout<<"WebRtcNs_Init fail\n";
		return -1;
	}
	status = WebRtcNs_set_policy(nsHandle, ns_level_);
	if (status != 0) {
		std::cout<<"WebRtcNs_set_policy fail\n";
		return -1;
	}
	three_bands_filter_48k = new webrtc::ThreeBandFilterBank(480);
	
	return 0;
}

inline void WebRtc_NS::Process(float * _data_in, float * _data_out, size_t data_size) {
	for (size_t nFrames = 0; nFrames < data_size / num_per_frame_ ;  nFrames++)
	{
		frame_process(_data_in + nFrames * num_per_frame_, _data_out + nFrames * num_per_frame_);
	}
	//std::cout << "Pass";
}

inline void WebRtc_NS::frame_process(float * _data_in, float * _data_out) {
	// process per frame should be 10ms, different sample rate requires different points per frame.
	// Max input size is 320 points for 32 khz
	// As to unify,160 points per band is suggested.
	if (num_per_frame_ == 80 || num_per_frame_ == 160)
	{
		float *input_buffer[1] = { _data_in };				//ns input buffer [band][data]   band:1~2 [L H]
		float *output_buffer[1] = { _data_out };			//ns output buffer [band][data]  band:1~2 [L H]
		WebRtcNs_Analyze(nsHandle, input_buffer[0]);
		WebRtcNs_Process(nsHandle, (const float *const *)input_buffer, 1, output_buffer);	// num_bands = 1 or 2 
		return;
	}
	else if (num_per_frame_ == 320) {
		// DONE: add a slpltting filter for 32k sample rate
		// Note: 
		// QMF's input is int16_t while WebRtcNs supports float data in this version,so there are some extra conversion.

		int16_t data_twobands_int16[2][160]{ { 0 } ,{ 0 } };
		int16_t data_in_int16[320]{};
		webrtc::FloatToS16(_data_in, 320, data_in_int16);
		// analysis
		WebRtcSpl_AnalysisQMF(data_in_int16, 320, data_twobands_int16[0], data_twobands_int16[1], TwoBands.analysis_state1, TwoBands.analysis_state2);
		// Two bands process
		float data_in_twobands_f[2][160] = { { 0 },{ 0 } };				//ns input buffer [band][data]   band:1~2 [L H]
		float data_out_twobands_f[2][160] = { { 0 },{ 0 } };			//ns output buffer [band][data]  band:1~2 [L H]

		webrtc::S16ToFloat(data_twobands_int16[0], 160, data_in_twobands_f[0]);
		webrtc::S16ToFloat(data_twobands_int16[1], 160, data_in_twobands_f[1]);

		float *input_buffer[2] = { data_in_twobands_f[0],data_in_twobands_f[1] };
		float *output_buffer[2] = { data_out_twobands_f[0],data_out_twobands_f[1] };
		// noise supression analyze and process.
		WebRtcNs_Analyze(nsHandle, input_buffer[0]);
		WebRtcNs_Process(nsHandle, input_buffer, 2, output_buffer);	// num_bands = 1~3 ,in 32k mode, num_bands = 2

		webrtc::FloatToS16(output_buffer[0], 160, data_twobands_int16[0]);
		webrtc::FloatToS16(output_buffer[1], 160, data_twobands_int16[1]);
		// synthesis
		WebRtcSpl_SynthesisQMF(data_twobands_int16[0], data_twobands_int16[1], 160, data_in_int16, TwoBands.synthesis_state1, TwoBands.synthesis_state2);
		webrtc::S16ToFloat(data_in_int16, 320, _data_out);
		return;
	}
	else if (num_per_frame_ == 480 || num_per_frame_ == 441)
	{
		// DONE: add a slpltting filter for 48k sample rate

		float band_in[3][160]{ {},{},{} };
		float band_out[3][160]{ {},{},{} };
		float * three_band_in[3] = { band_in[0],band_in[1],band_in[2] };
		float * three_band_out[3] = { band_out[0],band_out[1],band_out[2] };
		three_bands_filter_48k->Analysis(_data_in, 480, three_band_in);
		WebRtcNs_Analyze(nsHandle, three_band_in[0]);
		WebRtcNs_Process(nsHandle, three_band_in, 3, three_band_out);	// num_bands = 3 for 48khz
		three_bands_filter_48k->Synthesis(three_band_out, 160, _data_out);
		return;
	}
	else {
		std::cout << "Only support up to 48000khz";
		return;
	}

}

WebRtc_NS::WebRtc_NS()
{
}


WebRtc_NS::~WebRtc_NS()
{
	//delete[] nsHandle;
	WebRtcNs_Free(nsHandle);
	//delete[] three_bands_filter_48k;
}