#include <stdio.h>
#include <stdlib.h>
#include <math.h>
// #include <values.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <algorithm>
#include <unistd.h>

long geracoes = 100; //numero de geracoes
const int tam_pop = 200; //tamanho da populacao/swarm
const int tam_rep = tam_pop; //tamanho do repositorio
const int dimensoes_obj=2; //numero de objetivos
const int dimensoes_var=dimensoes_obj+10-1; //numero de variaveis de decisao (valor padrao do DTLZ2)
double limiteInferior=0, limiteSuperior=1; //todas as solucoes devem estar entre 0 e 1 (padrao do DTLZ2)
int objAtual=0; //usada para ordenar as solucoes de acordo com cada objetivo de forma simples no calculo da crowding distance

double alfa=0.1; //inercia
double beta=1.5; //influencia do melhor pessoal - componente cognitivo
double gama=0; //influencia do melhor local -- nao vamos usar
double delta=1.5; //influencia do melhor global
double epsilon=1; //aumento ou diminuicao da velocidade da particula

struct Individuo {
    double x[dimensoes_var]; //vetor de variaveis de decisao
    double fx[dimensoes_obj]; //vetor de objetivos
    double cd=0; //opcional para o controle do repositorio
    bool valida=false;//opcional para o controle do repositorio
};

struct Particula {
        Individuo solucao; //solucao atual da particula
        double velocidade[dimensoes_var]; //vetor de velocidade da particula
        Individuo melhorPessoal; //melhor solucao encontrada ate agora por essa particula
        Individuo melhorLocal; //melhor solucao encontrada ate agora pelos vizinhos dessa particula -- nao vamos usar, por isso gama e 0
        Individuo melhorGlobal; //uma das solucoes do repositorio
};

Particula populacao[tam_pop]; //Populacao ou enxame
Individuo repositorio[tam_rep+1]; //repositorio ou arquivo externo - sempre mais 1, porque ele estrapola a capacidade em 1 de vez em quando

//opcional
int tamanhoAtualRepositorio=0; //armazena o numero de solucoes validas no repositorio

//prototipacao das funcoes
void inicializacao(); //inicializa todas as solucoes aleatoriamente
void aptidao(Individuo *ind); //calcula o fitness ou valor objetivo
void atualizarArquivo(); //com os passos de atualizacao dos lideres e avaliacao de qualidade
void selecionarLider(Particula *part); //seleciona o lider global para cada individuo
void calcularVelocidade(Particula *part); //calcula a nova velocidade
void atualizarPosicao(Particula *part); //calcula a nova posicao
void mutacao(Individuo *ind); //mutacao, acontece em cada variavel de decisao de cada individuo com uma dada probabilidade
void atualizarMelhorPessoal(Particula *part); //atualiza a melhor solucao ja encontrada pela propria particula

int indice_disponivel();
bool verifica_repositorio();
bool dominados(Individuo IndPop, Individuo IndRep);
bool nao_dominados(Individuo IndPop, Individuo IndRep);

void calcularDTLZ2(double* x, double* fx);
void calcularDTLZ3(double* x, double* fx);

int main(const int argc, const char* argv[]){
    srand (time(NULL)); //inicializa a semente aleatória com o tempo atual
    
    inicializacao();
    for(int i=0;i<tam_pop;i++){
        aptidao(&populacao[i].solucao);
        memcpy(&populacao[i].melhorPessoal, &populacao[i].solucao, sizeof(Individuo));//inicializa o melhor pessoal
    }
    atualizarArquivo();
    
    for(long g=0;g<geracoes;g++){//laco principal
        for(int i=0;i<tam_pop;i++){ //laco para atualizacao dos individuos
            selecionarLider(&populacao[i]);
            calcularVelocidade(&populacao[i]);
            atualizarPosicao(&populacao[i]);
            mutacao(&populacao[i].solucao);
            aptidao(&populacao[i].solucao);
            atualizarMelhorPessoal(&populacao[i]);
        }
        atualizarArquivo();
    }
    for(int i=0;i<tam_rep+1;i++){
        if(repositorio[i].valida){
            for(int j=0;j<dimensoes_obj;j++)
                printf("%.3f ", repositorio[i].fx[j]);
                printf(" -- ");
                for(int j=0;j<dimensoes_var;j++){
                    printf("%.3f ", repositorio[i].x[j]);
                }
            printf("\n");
        }
    }
}

void inicializacao(){
    double extensao = limiteSuperior - limiteInferior; // qual o tamanho total
    for(int i=0;i<tam_pop;i++){
        for(int j=0;j<dimensoes_var;j++){
            populacao[i].solucao.x[j]=limiteInferior+(rand()/(double)RAND_MAX)*extensao; //limite inferior + aleatorio na extensao
            populacao[i].velocidade[j]=limiteInferior+(rand()/(double)RAND_MAX)*extensao; //limite inferior + aleatorio na extensao
        }
    }
}

void aptidao(Individuo *ind){
    for(int i=0;i<tam_pop;i++){
        calcularDTLZ2(ind->x, ind->fx);
//         calcularDTLZ3(ind->x, ind->fx);
    }
}

void atualizarArquivo(){
    //se o repositorio esta vazio, adiciona qualquer solucao
    if (verifica_repositorio() == true){
        repositorio[indice_disponivel()] = populacao[1].solucao;
    }else{
        //laco mais interno | corre toda populacao
        for(int ind_pop=0; ind_pop<tam_pop; ind_pop++){
            //controle de insercao de novas solucoes
            bool _alfa = false;
            //laco externo | corre o repositorio 
            for (int ind_rep=0; ind_rep<tam_rep; ind_rep++){
                // se IndPOP for dominada por IndREP
                if (dominados(populacao[ind_pop].solucao, repositorio[ind_rep]) == true){
                    break;
                // se IndPOP nao for dominada por IndREP
                }else {
                    // se a solucão ainda não tiver sido inserida no repositorio
                    if (_alfa != true){
                        repositorio[indice_disponivel()] = populacao[ind_pop].solucao;
                        tamanhoAtualRepositorio ++;
                        //ganrante que ela não será inserida novamente
                        _alfa = true;
                    // se IndPOP não for uma "nao-dominada" logo ela domina IntREP
                    if (nao_dominados(populacao[ind_pop].solucao, repositorio[ind_rep]) == false){
                        
                    } 
                    if (tamanhoAtualRepositorio >= tam_rep +1){

                    }  
                }
            }
        }
    }
    //caso contrario
        //verifica se a nova solucao e dominada por alguma do repositorio ou igual a alguma do repositorio
        //se nao for
            //verifica se a nova solucao domina alguma que ja esta no repositorio, se dominar, remove
            //adiciona a nova solucao ao repositorio
            //se o tamanho do repositorio estiver maior que o tamanho maximo permitido (e possivel extrapolar temporariamente o tamanho em uma solucao) remove uma solucao
    
    //ao final, calcula a crowding distance das solucoes presentes no repositorio
}

//verifica o indice disponivel no repositorio
int indice_disponivel(){
    int indice = 0;
    for(int a=0; a <= tam_rep; a++){
        if (repositorio[a].valida == 0){
            indice = a;
            break;
        }
    }
    return indice;
}


bool verifica_repositorio(){
    int contador = 0;
    for(int a=0; a <= tam_rep; a++){
        if (repositorio[a].valida == 0)
        contador ++;
    }
    if (contador == tam_rep + 1){
        return true;
    }else{
        return false;
    }
}

bool dominados(Individuo IndPop, Individuo IndRep){
    //variavel contadora de objetivosß
    int cont = 0;
    for (int obj=0; obj < dimensoes_obj; obj++){
        if (IndPop.fx[obj] <= IndRep.fx[obj]){
            //incrementanda toda vez que o obetivo de IndPop for menor ou igual que objetivo de IndRep
            cont ++;
        }
    }
    if (cont == dimensoes_obj){
        // se o valor de cont for o mesmo que a quantidade de objetivos, quer dizer que todas as solucoes de IndPop
        // sao domindadas por IndRep | se for isso, retorna TRUE
        return true;
    }else{
        return false;
    }
}


bool nao_dominados(Individuo IndPop, Individuo IndRep){
    int cont_IndPop=0;
    int cont_IndRep=0;
    for (int obj=0; obj < dimensoes_obj; obj++){
        if (IndPop.fx[obj] > IndRep.fx[obj]){
            cont_IndPop ++;
        }else if (IndPop.fx[obj] < IndRep.fx[obj]){
            cont_IndRep ++;
        }
    }
    if (cont_IndRep != 0 && cont_IndPop != 0){
        return true;
    }else{
        return false;
    }
}


void selecionarLider(Particula *part){
    //torneio binario entre solucoes do repositorio
    //pega duas solucoes aleatorias do repositorio, a que tiver a maior crowding distance vence
}

void calcularVelocidade(Particula *part){
    for(int j=0;j<dimensoes_var;j++){
        double b = (rand()/(double)RAND_MAX) * beta;
        double c = (rand()/(double)RAND_MAX) * gama;
        double d = (rand()/(double)RAND_MAX) * delta;
        double vel=0;
        vel+=alfa*part->velocidade[j];
        vel+=b*(part->melhorPessoal.x[j]-part->solucao.x[j]);
        vel+=c*(part->melhorLocal.x[j]-part->solucao.x[j]);
        vel+=d*(part->melhorGlobal.x[j]-part->solucao.x[j]);
        part->velocidade[j]=vel;
    }
}

void atualizarPosicao(Particula *part){
    for(int j=0;j<dimensoes_var;j++){
        part->solucao.x[j]+=epsilon*part->velocidade[j];
        if(part->solucao.x[j] > limiteSuperior)
            part->solucao.x[j] = limiteSuperior;
        if(part->solucao.x[j] < limiteInferior)
            part->solucao.x[j] = limiteInferior;
    }
}

void mutacao(Individuo *ind){
    double probabilidade=1.0/dimensoes_var;
	double distributionIndex=30.0;
	double rnd, delta1, delta2, mut_pow, deltaq;
	double y, yl, yu, val, xy;
    
    //Mutacao polinomial
    for (int var=0; var < dimensoes_var; var++) {
        if ((rand()/(double)RAND_MAX) <= probabilidade){
            y      = ind->x[var];
            yl     = limiteInferior;
            yu     = limiteSuperior;
            delta1 = (y-yl)/(yu-yl);
            delta2 = (yu-y)/(yu-yl);
            rnd=(rand()/(double)RAND_MAX);
            mut_pow = 1.0/(distributionIndex+1.0);
            if (rnd <= 0.5){
                xy     = 1.0-delta1;
                val    = 2.0*rnd+(1.0-2.0*rnd)*(pow(xy,(distributionIndex+1.0)));
                deltaq =  pow(val,mut_pow) - 1.0;
            }
            else{
                xy = 1.0-delta2;
                val = 2.0*(1.0-rnd)+2.0*(rnd-0.5)*(pow(xy,(distributionIndex+1.0)));
                deltaq = 1.0 - (pow(val,mut_pow));
            }
            y = y + deltaq*(yu-yl);
            if (y<yl)
                y = yl;
            if (y>yu)
                y = yu;
            ind->x[var]= y;
        }
    }
}

void atualizarMelhorPessoal(Particula *part){
    //se o melhor pessoal nao dominar a nova solução, troca
}

void calcularDTLZ2(double* x, double* fx){
	int k= dimensoes_var - dimensoes_obj + 1;
	double g=0.0;

	for(int i=dimensoes_var-k;i<dimensoes_var;i++)
		g+=(x[i]-0.5)*(x[i]-0.5);

	for(int i=0;i<dimensoes_obj;i++)
		fx[i] = (1.0+g);

	for(int i=0;i<dimensoes_obj;i++){
		for(int j=0;j<dimensoes_obj-(i+1);j++)
			fx[i] *= cos(x[j]*0.5*M_PI);
        if(i != 0){
            int aux = dimensoes_obj - (i+1);
            fx[i] *= sin(x[aux]*0.5*M_PI);
        }
	}
}

void calcularDTLZ3(double* x, double* fx){
	int k= dimensoes_var - dimensoes_obj + 1;
	double g=0.0;

	for(int i=dimensoes_var-k;i<dimensoes_var;i++)
		g+=(x[i]-0.5)*(x[i]-0.5)-cos(20.0*M_PI*(x[i]-0.5));

	g=100.0*(k+g);
	for(int i=0;i<dimensoes_obj;i++)
		fx[i] = 1.0+g;

	for(int i=0;i<dimensoes_obj;i++){
		for(int j=0;j<dimensoes_obj-(i+1);j++)
			fx[i] *= cos(x[j]*0.5*M_PI);
			if(i != 0){
				int aux = dimensoes_obj - (i+1);
				fx[i] *= sin(x[aux]*0.5*M_PI);
			}
	}
}