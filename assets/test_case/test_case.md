## Test case

input :

assets\babble_15dB.wav

output:

ns_level = kLow    	||	webrtc_ns_kLow_babble_15dB.wav
ns_level = kModerate	||	webrtc_ns_kModerate_babble_15dB.wav
ns_level = kHigh		||	webrtc_ns_kHigh_babble_15dB.wav

The rnnoise and speexdsp's noise suppression test case wav files are captured from the https://people.xiph.rog/~jm/demo/rnnoise/

Thses files can be found in the assets\test_case .

## Performance

With the same input,babble_15dB.wav, we set up a contrast among RNNoise, speexdsp and webrtc_ns.In the listening test, RNNoise performs well and suppresses the noise in babble.. It introduces a bit of artifacts which sounds unnature but it is tolerable. 

For the webrtc's ns, kHigh performs better than kModerate  which can suppress the whole noise.The kLow case has a poor performance than the others. Notes that the webrtc's ns has an impacts of the Gain when processed the audio data, maybe it should be interfaced with AGC module. 

Finally, speexdsp's ns can suppress the babble noise but  not completely reduce the backgourd noise. These noise suppression algorithm performance can be summed up  as blows:
$$
kLow   < speechx \approx kModerate <kHigh \approx Rnnoise
$$

