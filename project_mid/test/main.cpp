// sudo mbed compile --source . --source ~/ee2405/mbed-os-build/ -m K66F -t GCC_ARM -f --profile audio.json 
#include "accelerometer_handler.h"
#include "config.h"
#include "magic_wand_model_data.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include "DA7212.h"
#include <cmath>
#include <string>
#include "mbed.h"
#include "uLCD_4DGL.h"

uLCD_4DGL ulcd(D1, D0, D2);
// Serial pc(USBTX, USBRX);
EventQueue queue(32* EVENTS_EVENT_SIZE);
Thread t;
InterruptIn pause(SW2);
InterruptIn confirm(SW3);

int mode = false, test = -1, song_num=0, Sel_ = 0; // 0->mode/ 1->song;
int PredictGesture(float*);
// int16_t waveform[kAudioTxBufferSize]
// void playNote(int);
// DA7212 audio;

void mode_selection(){
	mode = true;
    test = -1;
}

void choose(){
    if(test == 2 && Sel_ == 0){
        Sel_ = 1;
        test = -1;
    }else{ // test = 0,1 / song_select
        Sel_ = 0;
        mode = false;
    }

}

struct song_list{
  int *song;
  int *length;
  int size;
  void init(int s){
    song = new int [s];
    length = new int [s];
    size = s;
  }
};

int main(){
	pause.rise(mode_selection);
	confirm.rise(choose);
  string name[total_song]={"Little_Star", "Bee"};
  song_list songList[total_song];
  songList[0].init(42);
  songList[1].init(49);
  for(int i=0; i<songList[0].size; i++){
    songList[0].song[i] = song_1[i];
    songList[0].length[i] = Length_1[i];
  }
  for(int i=0; i<songList[1].size; i++){
    songList[1].song[i] = song_2[i];
    songList[1].length[i] = Length_2[i];
  }

  t.start(callback(&queue, &EventQueue::dispatch_forever));
	constexpr int kTensorArenaSize = 60 * 1024;
  uint8_t tensor_arena[kTensorArenaSize];
	bool should_clear_buffer = false;
  bool got_data = false;
	int gesture_index;
  static tflite::MicroErrorReporter micro_error_reporter;
  tflite::ErrorReporter* error_reporter = &micro_error_reporter;
  const tflite::Model* model = tflite::GetModel(g_magic_wand_model_data);

  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report(
      "Model provided is schema version %d not equal "
      "to supported version %d.",
      model->version(), TFLITE_SCHEMA_VERSION);
    return -1;
  }
 	static tflite::MicroOpResolver<6> micro_op_resolver;
 
  	micro_op_resolver.AddBuiltin(
    tflite::BuiltinOperator_RESHAPE,
    tflite::ops::micro::Register_RESHAPE(),1);

  	micro_op_resolver.AddBuiltin(
      tflite::BuiltinOperator_DEPTHWISE_CONV_2D,
      tflite::ops::micro::Register_DEPTHWISE_CONV_2D());

  	micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_MAX_POOL_2D,
                               tflite::ops::micro::Register_MAX_POOL_2D());

  	micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_CONV_2D,
                               tflite::ops::micro::Register_CONV_2D());

  	micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_FULLY_CONNECTED,
                               tflite::ops::micro::Register_FULLY_CONNECTED());

  	micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_SOFTMAX,
                               tflite::ops::micro::Register_SOFTMAX());


  	static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  	tflite::MicroInterpreter* interpreter = &static_interpreter;

  	// Allocate memory from the tensor_arena for the model's tensors
  	interpreter->AllocateTensors();
  	// Obtain pointer to the model's input tensor
  	TfLiteTensor* model_input = interpreter->input(0);

  	if ((model_input->dims->size != 4) || (model_input->dims->data[0] != 1) ||
      (model_input->dims->data[1] != config.seq_length) ||
      (model_input->dims->data[2] != kChannelNumber) ||
      (model_input->type != kTfLiteFloat32)) {

        error_reporter->Report("Bad input tensor parameters in model");
        return -1;
  	}
  	int input_length = model_input->bytes / sizeof(float);
	TfLiteStatus setup_status = SetupAccelerometer(error_reporter);
	if (setup_status != kTfLiteOk) {
    	error_reporter->Report("Set up failed\n");
    	return -1;
  	}
	error_reporter->Report("Set up successful...\n");

	// --------- //
	while(true){
		if(mode && Sel_ == 0){
			if(test == -1){
				ulcd.cls();
				ulcd.printf("<<\n>>\nchange_song\n");
        ulcd.locate(15,0);
        ulcd.printf("v");
				test ++;
      }
            
			got_data = ReadAccelerometer(error_reporter, model_input->data.f,input_length, should_clear_buffer);
    	if (!got_data) {
				should_clear_buffer = false;
				continue;
    	}
    	TfLiteStatus invoke_status = interpreter->Invoke();
  		if (invoke_status != kTfLiteOk) {
				error_reporter->Report("Invoke failed on index: %d\n", begin_index);
  			continue;
	    }
	    gesture_index = PredictGesture(interpreter->output(0)->data.f);

	    should_clear_buffer = gesture_index < label_num;
	    if (gesture_index < label_num && gesture_index == 2) {
        ulcd.locate(15,test);
        ulcd.printf(" ");
        test = (test+1)%3;
        ulcd.locate(15,test);
        ulcd.printf("v");
    	}
      ulcd.locate(0,15);
      ulcd.printf("sel=%d, flag=%d",Sel_,test);
		}else if(mode && Sel_ == 1 ){
      if(test == -1){
        ulcd.cls();
        for(int i=0; i<2; i++){
          string temp = name[i]; //input[i].song_name;
          char ch[temp.size()+1];
          strcpy(ch,temp.c_str());
          for(int i=0; i<strlen(ch); i++){
            ulcd.putc(ch[i]);
          }
          ulcd.printf("\n");
        }
          ulcd.locate(12,0);
          ulcd.printf("v");
          song_num = 0, test=2;
      }
      got_data = ReadAccelerometer(error_reporter, model_input->data.f,input_length, should_clear_buffer);
    	if (!got_data) {
				should_clear_buffer = false;
	  		continue;
  		}
  		TfLiteStatus invoke_status = interpreter->Invoke();
    	if (invoke_status != kTfLiteOk) {
				error_reporter->Report("Invoke failed on index: %d\n", begin_index);
				continue;
	    }
	    gesture_index = PredictGesture(interpreter->output(0)->data.f);

		  should_clear_buffer = gesture_index < label_num;
		  if (gesture_index < label_num && gesture_index == 2) {
        ulcd.locate(12,song_num);
        ulcd.printf(" ");
        song_num = (song_num+1)%total_song;
        ulcd.locate(12,song_num);
        ulcd.printf("v");
    	}
   
    } else{
      if(test != -1){
        ulcd.cls();
        if(test == 0 && song_num != 0){
          song_num -= 1;
        }else if(test == 1 && song_num != total_song){
          song_num +=1;
        }
        test = -1;
      }
			ulcd.locate(0,0);
			ulcd.printf("song info--\nname:");
      string temp = name[song_num];//input[song_num].song_name;
      char ch[temp.size()+1];
      strcpy(ch, temp.c_str());
      for(int i=0; i<strlen(ch); i++){
        ulcd.putc(ch[i]);       
      }
      ulcd.locate(0,15);
      ulcd.printf("sel=%d, mode=%d",Sel_,mode);
      
      for(int i = 0; i < songList[song_num].size; i++){
        int length = songList[song_num].length[i];
        while(length--){
          // queue.call(playNote, songList[song_num].song[i]);
          // if(length <= 1) wait(1.0);
        }
      }
    }
	}
}

int PredictGesture(float* output) {
  static int continuous_count = 0;
  static int last_predict = -1;
  int this_predict = -1;

  for (int i = 0; i < label_num; i++) {
    if (output[i] > 0.8) this_predict = i;
  }
  // No gesture was detected above the threshold
  if (this_predict == -1) {
    continuous_count = 0;
    last_predict = label_num;
    return label_num;
  }


  if (last_predict == this_predict) {
    continuous_count += 1;
  } else {
    continuous_count = 0;
  }
  last_predict = this_predict;

  if (continuous_count < config.consecutiveInferenceThresholds[this_predict]) {
    return label_num;
  }

  continuous_count = 0;
  last_predict = -1;
  return this_predict;
}
// void playNote(int freq){

//   for (int i = 0; i < kAudioTxBufferSize; i++){
//     waveform[i] = (int16_t) (sin((double)i * 2. * M_PI/(double) (kAudioSampleFrequency / freq)) * ((1<<16) - 1));
//   }

//   // the loop below will play the note for the duration of 1s
//   for(int j = 0; j < kAudioSampleFrequency / kAudioTxBufferSize; ++j){
//     audio.spk.play(waveform, kAudioTxBufferSize);
//   }
// }