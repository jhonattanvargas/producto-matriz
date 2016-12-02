#include <cstdlib>
#include <node.h>
#include <iostream>
#include <v8.h>
#include <cmath>
#include <pthread.h>
#include <thread>
using namespace std;

using v8::Exception;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::String;
using v8::Value;
using v8::Array;
using v8::Integer;

template <typename T>
bool
my_isnan(const T x)
{
#if __cplusplus >= 201103L
	  using std::isnan;
#endif
	    return isnan(x);
}

#define BILLION  1E9;

float **A;
float **B;
float **result;
int task_per_thread;
int n = 0, m = 0, p = 0, q = 0;

void setMatrizA(Local<Array>);
void setMatrizB(Local<Array>);
void mostrarMatriz(float**, int, int);

void *runner(void *pid) {

	int slice = (long) pid;
	int to = task_per_thread*slice;
	int from = (task_per_thread*(slice+2)<n) ? task_per_thread*(slice+1) : n;
	//std::cout << "to: "<< to<< "from: "<< from << std::endl;
	for (unsigned int i = to; i < from ; ++i){
    for (unsigned int j = 0; j < q ; ++j){
			result[i][j] = 0;
      for (unsigned int k = 0; k < m ; k++) {
        result[i][j]  += (A[i][k] * B[k][j]);
      }
    }
  }
  pthread_exit(NULL);
}

void parallelProduct(const FunctionCallbackInfo<Value>& args){
	//////////////////////////////////////////
	//Inicio código para leer las dos matrices
	//////////////////////////////////////////
  Isolate* isolate = args.GetIsolate();
	if(args.Length() < 2){
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Numero equivocado de argumentos")));
    return;
  }

	if(args[0]->IsArray() && args[1]->IsArray()){

    Local<Array> a = Local<Array>::Cast(args[0]);
    setMatrizA(a);
		//mostrarMatriz(A,n,m);
    Local<Array> b = Local<Array>::Cast(args[1]);
		setMatrizB(b);
		//mostrarMatriz(B,p,q);
  }
	std::cout << "creando matriz para resultado." << std::endl;
	result = (float**) malloc(n *sizeof(float*));
	for (unsigned int i = 0; i < n; i++) {
		result[i] = (float*) malloc(q *sizeof(float));
	}
	//////////////////////////////////////////
	//Fin código para leer las dos matrices///
	//////////////////////////////////////////

	unsigned cpus = std::thread::hardware_concurrency();
	//std::cout << "cpus: "<< cpus << std::endl;
	pthread_t threads[cpus];
	task_per_thread = n / cpus;
	int rc;

	std::cout << "calculando producto..." << std::endl;
	struct timespec requestStart, requestEnd;
	//inicio tiempo de medida
	clock_gettime(CLOCK_REALTIME, &requestStart);
	for(int i=0; i < cpus; i++ ){
    rc = pthread_create(&threads[i], NULL, runner, (void *) i);
    if (rc){
       cout << "Error:unable to create thread," << rc << endl;
       exit(-1);
    }
  }

  for(int i=0; i < cpus; i++ ){
    pthread_join(threads[i],NULL);
  }
	//fin tiempo de medida
  clock_gettime(CLOCK_REALTIME, &requestEnd);
  double accum = ( requestEnd.tv_sec - requestStart.tv_sec )
      + ( requestEnd.tv_nsec - requestStart.tv_nsec )
      / BILLION;
  printf( "Parallel Time taken: %lf\n", accum );

	Local<Array> array = Array::New(isolate);

	std::cout << "creando objeto ... " << std::endl;
	for (int i = 0; i < n; i++) {
		Local<Array> aux = Array::New(isolate);
		for (int j = 0; j < q; j++) {
			aux->Set(j,Integer::New(isolate,result[i][j]));
		}
		array->Set(i,aux);
	}

	free(A);
	free(B);
	free(result);

	Local<Object> obj = Object::New(isolate);
	obj->Set(String::NewFromUtf8(isolate, "cpus"), Number::New(isolate, cpus));
  obj->Set(String::NewFromUtf8(isolate, "time"), Number::New(isolate, accum));
	obj->Set(String::NewFromUtf8(isolate, "product"),array );

	std::cout << "done" << std::endl;

	args.GetReturnValue().Set(obj);
}

void serialProduct(const FunctionCallbackInfo<Value>& args){

  Isolate* isolate = args.GetIsolate();

  if(args.Length() < 2){
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Numero equivocado de argumentos")));
    return;
  }

  if(args[0]->IsArray() && args[1]->IsArray()){

		Local<Array> a = Local<Array>::Cast(args[0]);
    setMatrizA(a);
		//mostrarMatriz(A,n,m);
    Local<Array> b = Local<Array>::Cast(args[1]);
		setMatrizB(b);
		//mostrarMatriz(B,p,q);
  }

  result = (float**) malloc(n *sizeof(float*));
  for (unsigned int i = 0; i < n; i++) {
    result[i] = (float*) malloc(q *sizeof(float));
  }
  std::cout << "calculando producto..." << std::endl;
  struct timespec requestStart, requestEnd;
  //inicio tiempo de medida
  clock_gettime(CLOCK_REALTIME, &requestStart);

  for (unsigned int i = 0; i < n ; ++i){
    for (unsigned int j = 0; j < q ; ++j){
      result[i][j] = 0;
      for (unsigned int k = 0; k < m ; k++) {
        result[i][j]  += (A[i][k] * B[k][j]);
      }
    }
  }

  //fin tiempo de medida
  clock_gettime(CLOCK_REALTIME, &requestEnd);
  double accum = ( requestEnd.tv_sec - requestStart.tv_sec )
      + ( requestEnd.tv_nsec - requestStart.tv_nsec )
      / BILLION;
  printf( "Serial Time taken: %lf\n", accum );

  Local<Array> array = Array::New(isolate);

	//mostrarMatriz(result,n,q);

  std::cout << "creando objeto ... " << std::endl;
  for (int i = 0; i < n; i++) {
    Local<Array> aux = Array::New(isolate);
    for (int j = 0; j < q; j++) {
      aux->Set(j,Integer::New(isolate,result[i][j]));
    }
    array->Set(i,aux);
  }

  free(A);
  free(B);
  free(result);

	Local<Object> obj = Object::New(isolate);
	obj->Set(String::NewFromUtf8(isolate, "cpus"), Number::New(isolate, 1));
	obj->Set(String::NewFromUtf8(isolate, "time"), Number::New(isolate, accum));
	obj->Set(String::NewFromUtf8(isolate, "product"),array );

	std::cout << "done" << std::endl;

	args.GetReturnValue().Set(obj);

}

void setMatrizA(Local<Array> a){
	std::cout << "leyendo matriz a ..." << std::endl;
	n = a->Length();
	A = (float**) malloc(a->Length() * sizeof(float*));
	for (int index = 0, size = a->Length(); index < size; index++) {
		Local<Value> element = a->Get(index);
		if (element->IsArray()) {
			Local<Array> b = Local<Array>::Cast(element);
			m = b->Length();
			A[index] = (float*) malloc(b->Length() * sizeof(float));
			for (int index2 = 0, size2 = b->Length(); index2 < size2; index2++) {
				Local<Value> element2 = b->Get(index2);
				String::Utf8Value arg(b->Get(index2)->ToString());
				if(!my_isnan(atof(*arg))){
					A[index][index2] = atof(*arg);
				}
			}
		}
	}
}

void setMatrizB(Local<Array> a2){
	std::cout << "leyendo matriz b ..." << std::endl;
	p = a2->Length();
	B = (float**) malloc(a2->Length() * sizeof(float*));
	for (int index = 0, size = a2->Length(); index < size; index++) {
		Local<Value> element = a2->Get(index);
		if (element->IsArray()) {
			Local<Array> b2 = Local<Array>::Cast(element);
			q = b2->Length();
			B[index] = (float*) malloc(b2->Length() * sizeof(float));
			for (int index2 = 0, size2 = b2->Length(); index2 < size2; index2++) {
				Local<Value> element2 = b2->Get(index2);
				String::Utf8Value arg(b2->Get(index2)->ToString());
				if(!my_isnan(atof(*arg))){
					B[index][index2] = atof(*arg);
				}
			}
		}
	}
}

void mostrarMatriz(float **mat, int n, int m){
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++) {
			std::cout << mat[i][j] << " | ";
		}
		std::cout << std::endl;
	}
}

void Init(Local<Object> exports) {
NODE_SET_METHOD(exports, "serialProduct", serialProduct);
NODE_SET_METHOD(exports, "parallelProduct", parallelProduct);
}

NODE_MODULE(addon, Init)
