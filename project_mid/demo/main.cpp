// sudo mbed compile --source . --source ~/ee2405/mbed-os-build/ -m K66F -t GCC_ARM -f --profile audio.json 
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include "accelerometer_handler.h"
#include "magic_wand_model_data.h"
#include "config.h"
#include "DA7212.h"
#include <cmath>
#include "mbed.h"
#include "uLCD_4DGL.h"
#define total_song 3
#define bufferLength (32)
const int Size = 50;

DA7212 audio;
int16_t waveform[kAudioTxBufferSize];
uLCD_4DGL ulcd(D1, D0, D2);
Serial pc(USBTX, USBRX);
Thread DNN(osPriorityNormal,120*1024);
InterruptIn pause(SW2);
InterruptIn confirm(SW3);

char serialInBuffer[bufferLength];
int mode = false, flag = -1, song_num = 0;
int song[Size*total_song],length[Size*total_song];

void mode_selection();
void choose();
void playNote(int);
void loadSignal(void);
void Thread_DNN();

int main(){
  DNN.start(Thread_DNN);
  pause.rise(mode_selection);
  confirm.rise(choose);

  ulcd.printf("loading...");
  loadSignal();

  while(true){
    // mode
    if(mode){
      playNote(0);
      ulcd.cls();
      ulcd.printf("<<\n>>\nchange song\ntiako game\n");
      ulcd.locate(15,0);
      ulcd.printf("v");
      flag ++;

      while (mode){
        ulcd.locate(15,flag);
        ulcd.printf("v");
        ulcd.locate(15, (flag+3)%4);
        ulcd.printf(" ");
        ulcd.locate(0,15);
        ulcd.printf("flag=%d, no.%d",flag,song_num);
      }
    // change song
    }else if(flag == 4){
      playNote(0);
      ulcd.cls();
      for(int i=0; i<total_song; i++){
        ulcd.printf("no.%d       \n",i);
      }
      ulcd.locate(12,song_num);
      ulcd.printf("v");

      while(flag == 4){
        ulcd.locate(12,song_num);
        ulcd.printf("v");
        ulcd.locate(12,(song_num+total_song-1)%total_song);
        ulcd.printf(" ");
      }
    // music
    } else{
      bool tic = false;
      int count = 0;
      if(flag != -1){
        ulcd.cls();
        if(flag == 0 && song_num != 0) song_num -= 1;
        else if(flag == 1 && song_num != total_song-1) song_num +=1;
        else if(flag == 3) tic = true;
        flag = -1;
      }
      ulcd.locate(0,0);
      ulcd.printf("song info--\nSong No.%d",song_num);

      for(int i = 0; i < Size; i++){
        if(!mode){
          int len = length[song_num*50+i], number = 0;
          int a[3]={0,0,0};
          a[0] = length[song_num*50+i];
          if( i+1 < Size) a[1] = length[song_num*50+i+1];
          if( i+2 < Size) a[2] = length[song_num*50+i+2];
          ulcd.locate(5,7);
          if(tic) ulcd.printf("%d__%d__%d\n",a[0],a[1],a[2]);
          while(len-- > 0){
            for(int j = 0; j < kAudioSampleFrequency / kAudioTxBufferSize; ++j){
              playNote(song[song_num*50+i]);
              if(!number)number = tilt();
            }
          }
          if(tic){
            if(a[0] == 0) ulcd.printf("\nscoring..."), count++, playNote(0);
            else if(number == 0) ulcd.printf("\nfail!   ");
            else if((number+a[0])%2 == 0) ulcd.printf("\ncorrect!"), count ++;
            else ulcd.printf("\nfail!   ");
          }
        }
      }
      if(tic){
        ulcd.cls();
        ulcd.locate(0,15);
        ulcd.printf("score:%d",count*2);
      }
    }
  }
}

void mode_selection(){
  mode = true;
  flag = -1;
}

void choose(){
  if(flag == 2) flag = 4;
  else if(flag == 4) flag = 5;
  mode = false;
}

void playNote(int freq){
  for (int i = 0; i < kAudioTxBufferSize; i++){
    waveform[i] = (int16_t) (sin((double)i * 2. * M_PI/(double) (kAudioSampleFrequency / freq)) * ((1<<16) - 1));
  }
  audio.spk.play(waveform, kAudioTxBufferSize);
}

void loadSignal(void){
  int i = 0;
  int serialCount = 0;
  audio.spk.pause();
  while(i < total_song*Size){
    if(pc.readable()){
      serialInBuffer[serialCount] = pc.getc();
      serialCount++;
      if(serialCount == 5){
        serialInBuffer[serialCount] = '\0';
        float temp = (float) atof(serialInBuffer);
        song[i] = temp*1000;
        serialCount = 0;
        i++;
      }
    }
  }
  i = 0;
  while(i < total_song*Size){
    if(pc.readable()){
      serialInBuffer[serialCount] = pc.getc();
      serialCount++;
      if(serialCount == 5){
        serialInBuffer[serialCount] = '\0';
        float temp = (float) atof(serialInBuffer);
        length[i] = temp*1000;
        serialCount = 0;
        i++;
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

void Thread_DNN(){
  constexpr int kTensorArenaSize = 60 * 1024;
  uint8_t tensor_arena[kTensorArenaSize];
  bool should_clear_buffer = false;
  bool got_data = false;
  int gesture_index = label_num;
  static tflite::MicroErrorReporter micro_error_reporter;
  tflite::ErrorReporter* error_reporter = &micro_error_reporter;
  const tflite::Model* model = tflite::GetModel(g_magic_wand_model_data);

  if (model->version() != TFLITE_SCHEMA_VERSION) {
    error_reporter->Report(
      "Model provided is schema version %d not equal "
      "to supported version %d.",
      model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }

  static tflite::MicroOpResolver<6> micro_op_resolver;
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_RESHAPE, tflite::ops::micro::Register_RESHAPE(),1);
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_DEPTHWISE_CONV_2D, tflite::ops::micro::Register_DEPTHWISE_CONV_2D());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_MAX_POOL_2D, tflite::ops::micro::Register_MAX_POOL_2D());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_CONV_2D, tflite::ops::micro::Register_CONV_2D());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_FULLY_CONNECTED, tflite::ops::micro::Register_FULLY_CONNECTED());
  micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_SOFTMAX, tflite::ops::micro::Register_SOFTMAX());
  static tflite::MicroInterpreter static_interpreter(model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  tflite::MicroInterpreter* interpreter = &static_interpreter;
  interpreter->AllocateTensors();
  TfLiteTensor* model_input = interpreter->input(0);

  if ((model_input->dims->size != 4) || (model_input->dims->data[0] != 1) ||
      (model_input->dims->data[1] != config.seq_length) ||
      (model_input->dims->data[2] != kChannelNumber) ||
      (model_input->type != kTfLiteFloat32)) {
        error_reporter->Report("Bad input tensor parameters in model");
    return;
  }
  int input_length = model_input->bytes / sizeof(float);
	TfLiteStatus setup_status = SetupAccelerometer(error_reporter);
	if (setup_status != kTfLiteOk) {
    	error_reporter->Report("Set up failed\n");
    	return;
  }
  error_reporter->Report("Set up successful...\n");
  while(true){
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

    if(gesture_index == 2){
      if(flag == 4) song_num = (song_num+1)%total_song;
      else if(mode) flag = (flag+1)%4;
    }
  }
}