#include<iostream>
#include<stdio.h>
#include<sstream>

#include <ilcplex/ilocplex.h>

//AINDA NAO ENTENDO BEM ESSES TYPEDEFS...
//nao coloquei nomes nos talhoes. 
//o quao boa prática é nesse projeto?
typedef IloArray<IloNumVarArray> IloNumVarMatrix;
typedef IloArray<IloNumVarMatrix> IloNumVar3Matrix;
typedef IloArray<IloNumVar3Matrix> IloNumVar4Matrix;

using namespace std;

int main(int argc, char *argv[]) { 
    //Declarar os conjuntos
    int L;                  //LOTES
    int V;                  //VEICULOS
    int T;                  //TALHOES
    int E;                  //EMPILHADEIRAS
    float C;                //TEMPO DE CARREGAMENTO

    int* LT;                //Total de lotes em um talhão a
    int** LE;               //Matriz bin. Se o lote i pertence ao talhão a
    float* Ti;              //Tempo de ida da fábrica ao lote i
    float* Tv;              //Tempo de volta do lote i para a fábrica
    float** DE;             //Tempo de deslocamento entre o talhão a e b

    //Leitura do arquivo
    FILE* fp = fopen(argv[1],"r");
    if( fp == NULL) {
        printf("Erro ao abrir o arquivo!\n");
        exit(1);
    }
    
    printf("\nVerificação dos dados:\n");
    //Alocação dos conjuntos
    fscanf(fp, "%d %d %d %d %f", &L, &V, &T, &E, &C);
    printf("Quantidade de Lotes (contando): \t\t %d\n", L);
    printf("Quantidade de Veículos: \t %d\n", V);
    printf("Quantidade de Talhões: \t\t %d\n", T);
    printf("Quantidade de Empilhadeiras: \t %d\n", E);
    printf("Tempo de carregamento: \t\t %.2f horas\n", C);

    LT = new int[T];
    LE = new int*[L]; for (int i=0; i<L; i++) LE[i] = new int[T];
    Ti = new float[L];
    Tv = new float[L];
    DE = new float*[T+1]; for (int i=0; i<T+1; i++) DE[i] = new float[T];

    //LT
    printf("\n\nLT: Total de lotes em cada talhão\n");
    for (int i=0; i<T; i++){
        fscanf(fp, "%d", &LT[i]);
        printf("Talhão %d:\t%d lotes\n", i+1, LT[i]);
    }

    //LE    
    printf("\n\nLE: A qual talhão pertence cada lote\n");
    for (int i=0; i<L; i++){
        for (int j=0; j<T; j++) {
            fscanf(fp, "%d", &LE[i][j]);
            if (LE[i][j]) printf("Lote %d:\t Talhão %d\n", i+1, j+1);
        }
    }

    //Ti
    printf("\n\nTempo de ida da fábrica a cada lote\n");
    for (int i=0; i<L; i++) {
        fscanf(fp, "%f", &Ti[i]);
        printf("Tempo da fábrica ao lote %d:\t %.2f horas\n", i+1, Ti[i]);
    }

    //Tv
    printf("\n\nTempo de volta do lote para a fábrica\n");
    for (int i=0; i<L; i++) {
        fscanf(fp, "%f", &Tv[i]);
        printf("Tempo do lote %d à fábrica:\t %.2f\n", i+1, Tv[i]);
    }

    //DE
    printf("\n\nTempo de deslocamento entre os talhões\n");
    for (int i=0; i<T; i++) {
        for (int j=0; j<T; j++) {  
            fscanf(fp, "%f", &DE[i][j]);
            printf("Tempo entre os talhões %d e %d:\t %.2f\n", i+1, j+1, DE[i][j]);
        }    
    }

    for (int j = 0; j < T; j++) {
        fscanf(fp, "%f", &DE[T][j]);
        //printf("Tempo da garagem ao talhão:\t %.2f\n", i+1, j+1, DE[i][j]);

    }

    fclose(fp);
    //L[0] = fabrica. L[L+1] = fabrica como nó final.

    //Declaração do ambiente e do modelo matemático
    IloEnv env;
	IloModel modelo(env);

    //Declaração das variáveis de decisão
   
    //x = Variavel binaria, indica se o veıculo k atende o lote j logo apos i
    //x[veiculo][lotei][lotej]
    IloNumVar3Matrix x(env, V);
    for (int v = 0; v < V; ++v) {
        x[v] = IloNumVarMatrix(env, L+2);
        for (int i = 0; i < L+2; ++i) {
            x[v][i] = IloNumVarArray(env, L+2, 0, 1, ILOBOOL);
            
            for (int j = 0; j < L+2; ++j) modelo.add(x[v][i][j]);
        } 
    }

    //s = Variavel binaria, indica se o veıculo k atende o lote i
    //s[veiculo][lote]
    IloNumVarMatrix s(env, V);
    for (int v = 0; v < V; ++v) {
        s[v] = IloNumVarArray(env, L+2, 0, 1, ILOBOOL);
        for (int i = 0; i < L+2; ++i) modelo.add(s[v][i]);
    }

    //b = Instante em que o veıculo k chega no lote i
    //b[veiculo][lote]
    IloNumVarMatrix b(env, V);
    for (int v = 0; v < V; ++v) {
        b[v] = IloNumVarArray(env, L, 0, IloInfinity, ILOFLOAT);
        for (int i = 0; i < L; ++i) modelo.add(b[v][i]);
    }
    //d = Instante que o veıculo k começa a ser carregado com o lote i
    //d[veiculo][lote]
    IloNumVarMatrix d(env, V);
    for (int v = 0; v < V; ++v) {
        d[v] = IloNumVarArray(env, L, 0, IloInfinity, ILOFLOAT);
        for (int i = 0; i < L; ++i) modelo.add(d[v][i]);
    }
    
    //w = Tempo de espera do veıculo k no talhao para ser carregado com o lote i
    //w[veiculo][lote]
    IloNumVarMatrix w(env, V);
    for (int v = 0; v < V; ++v) {
        w[v] = IloNumVarArray(env, L, 0, IloInfinity, ILOFLOAT);
        for (int i = 0; i < L; ++i) modelo.add(w[v][i]);
    }    
    
    //h = Instante que o lote i foi atendido
    //h[lote]
    IloNumVarArray h(env, L, 0, IloInfinity, ILOFLOAT);
    for (int i = 0; i < L; ++i) modelo.add(h[i]);

    //y = Variavel binaria, indica se a empilhadeira e atende o talhao b logo apos a
    //y[empilhadeira][talhao_a][talhao_b]
    IloNumVar3Matrix y(env, E);
    for (int e = 0; e < E; ++e) {
        y[e] = IloNumVarMatrix(env, T+2);
        for (int a = 0; a < T+2; ++a) {
            y[e][a] = IloNumVarArray(env, T+2, 0, 1, ILOBOOL);
            for (int b = 0; b < T+2; ++b) modelo.add(y[e][a][b]);
        }
    }

    //z = Variavel binaria, indica se a empilhadeira e atende o talhao a
    //z[empilhadeira][talhao]
    IloNumVarMatrix z(env, E);
    for (int e = 0; e < E; ++e) {
        z[e] = IloNumVarArray(env, T, 0, 1, ILOBOOL);
        for (int a = 0; a < T; ++a) modelo.add(z[e][a]);
    }
    
    //c = Instante em que a empilhadeira e começa a atender o talhao a
    //c[empilhadeira][talhao]
    IloNumVarMatrix c(env, E);
    for (int e = 0; e < E; ++e) {
        c[e] = IloNumVarArray(env, T, 0, IloInfinity, ILOFLOAT);
        for (int a = 0; a < T; ++a) modelo.add(c[e][a]);
    }
    
    //m = Instante de inıcio de atendimento do ultimo lote, em horas
    IloNumVar m(env, 0, IloInfinity, ILOFLOAT);
    modelo.add(m);
    
    //Declaração da função objetivo
    IloExpr fo(env);

    fo = m;
    modelo.add(IloMinimize(env, fo));

    //Restrições
    const double M = 100000.0; // Um número grande para as restrições lógicas

    //Expressão 2: Garante que o ultimo lote só podera ser atendido apos todos os anteriores
    for (int i = 0; i < L; ++i) {
        modelo.add(h[i] <= m);
    }
    
    //Expressao 3: Indica que cada lote deve ser atendido por exatamente um veiculo.
    for (int i = 0; i < L; ++i) {
        IloExpr expr(env);
        for (int k = 0; k < V; ++k) {
            expr += s[k][i];
        }
        modelo.add(expr == 1);
        expr.end();
    }

    //Expressao 4: assegura que se o veıculo k nao atende o lote i, nao pode atender nenhum outro lote imediatamente antes ou apos.
    // Conecta a visita do veículo (s) com a rota (x).
    for (int k = 0; k < V; ++k) {
        for (int i = 0; i < L; ++i) { // Para cada lote i
            IloExpr saida(env), chegada(env);

            // Soma todos os arcos de SAÍDA possíveis do lote i
            // (para outros lotes ou para a garagem final L+1)
            for (int j = 0; j < L; ++j) if (i != j) saida += x[k][i][j];
            saida += x[k][i][L + 1];

            // Soma todos os arcos de CHEGADA possíveis no lote i
            // (vindo de outros lotes ou da garagem inicial L)
            for (int j = 0; j < L; ++j) if (i != j) chegada += x[k][j][i];
            chegada += x[k][L][i];

            modelo.add(saida == s[k][i]);
            modelo.add(chegada == s[k][i]);

            saida.end();
            chegada.end();
        }
    }

    // Expressão 5: se veículo k atende o lote i, então ele deve estar conectado
    for (int k = 0; k < V; ++k) {
        for (int i = 1; i <= L; ++i) { // apenas lotes reais
            IloExpr conexao(env);

            // Arcos saindo de i
            for (int j = 0; j <= L+1; ++j) {
                if (j != i) {
                    conexao += x[k][i][j];
                }
            }

            // Arcos entrando em i
            for (int j = 0; j <= L+1; ++j) {
                if (j != i) {
                    conexao += x[k][j][i];
                }
            }

            // Se s[k][i] = 1, então conexao >= 1
            modelo.add(IloIfThen(env, s[k][i] == 1, conexao >= 1));

            conexao.end();
        }
    }

    //expressao 6 
    for (int k = 0; k < V; ++k) {
        IloExpr lhs(env); // lado esquerdo = transições
        for (int i = 1; i <= L; ++i) {
            for (int j = 1; j <= L; ++j) {
                if (i != j) {
                    lhs += x[k][i][j];
                }
            }
        }

        IloExpr rhs(env); // lado direito = soma(s[k][i]) - 1
        for (int i = 1; i <= L; ++i) {
            rhs += s[k][i];
        }
        rhs -= 1;

        modelo.add(lhs == rhs);

        lhs.end();
        rhs.end();
    }



    //Expressao 7: certifica que todos os veículos irao sair da fabrica 
    for (int k = 0; k < V; ++k) {
        IloExpr expr(env);
        for (int j = 0; j < L; ++j) { // Para cada lote j
            expr += x[k][L][j];
        }
        expr += x[k][L][L+1]; // Ou direto para o depósito de chegada
        modelo.add(expr == 1);
        expr.end();
    }

    //Expressao 8: certifica que todos os veículos retornem a fábrica 
    for (int k = 0; k < V; ++k) {
        IloExpr expr(env);
        for (int i = 0; i < L; ++i) { // Vindo de cada lote i
            expr += x[k][i][L + 1];
        }
        expr += x[k][L][L + 1]; // Ou vindo direto do depósito de partida
        modelo.add(expr == 1);
        expr.end();
    }

    // Expressão 9: Conservacao de fluxo para cada lote (veículos).
    for (int k = 0; k < V; ++k) {
        for (int i = 0; i < L; ++i) { // Para cada lote i
            IloExpr total_chegadas(env);
            IloExpr total_saidas(env);
            
            for (int j = 0; j < L + 2; ++j) {
                if (i != j) {
                    total_chegadas += x[k][j][i];
                    total_saidas += x[k][i][j];
                }
            }
            modelo.add(total_chegadas == total_saidas);
            total_chegadas.end();
            total_saidas.end();
        }
    }

    //expressão 10 do artigo
    for (int k = 0; k < V; ++k) {
        for (int i = 1; i < L; ++i) {
            modelo.add(h[i] >= d[k][i] + C - M * (1 - s[k][i]));
        }
    }

    //Expressao 11
    for (int k = 0; k < V; ++k) {
        for (int i = 0; i < L; ++i) { // Loop para lotes de 0 a L-1
            modelo.add(d[k][i] >= b[k][i]);
        }
    }

    
    // Expressão 12: Sincronização entre empilhadeira e caminhão.
    // O carregamento do lote i só começa após a empilhadeira preparar a carga 
    // no talhão a ao qual o lote i pertence.
    for (int i = 0; i < L; ++i) {
        for (int a = 0; a < T; ++a) {
            // A condição principal é verificada primeiro
            if (LE[i][a] == 1) {
                // Os loops de veículo e empilhadeira só são executados quando necessário
                for (int k = 0; k < V; ++k) {
                    for (int e = 0; e < E; ++e) {
                        // A restrição é ativada se s[k][i]=1 E z[e][a]=1.
                        modelo.add(d[k][i] >= c[e][a] + DE[a][a] - M * (2 - s[k][i] - z[e][a]));
                    }
                }
            }
        }
    }

    // Expressão 13 (Versão Corrigida para Evitar Inviabilidade)
    // Garante o sequenciamento de tempo correto entre lotes consecutivos em uma rota.
    for (int k = 0; k < V; ++k) {
        for (int i = 0; i < L; ++i) {
            for (int j = 0; j < L; ++j) {
                if (i != j) {
                    // A chegada em j (b[k][j]) deve ser apos a conclusao em i (h[i]),
                    // mais o tempo de volta do lote i (Tv[i]) e o tempo de ida para o lote j (Ti[j]).
                    // A restricao so e ativada se o arco x[k][i][j] for usado (igual a 1).
                    modelo.add(b[k][j] >= h[i] + Tv[i] + Ti[j] - M * (1 - x[k][i][j]));
                }
            }
        }
    }

    // Expressão 14: Define o tempo de chegada para o primeiro lote visitado
    // a partir da garagem.
    for (int k = 0; k < V; ++k) {
        for (int i = 0; i < L; ++i) { // Para cada lote i
            // b[k][i] >= Ti[i] * x[k][L][i]
            // Onde L é o índice da garagem de partida.
            modelo.add(b[k][i] >= Ti[i] * x[k][L][i]);
        }
    }

    // Expressão 15: Garante que cada talhão 'a' seja atendido por exatamente uma empilhadeira 'e'.
    for (int a = 0; a < T; ++a) { // Para cada talhão 'a'
        IloExpr expr(env);
        for (int e = 0; e < E; ++e) { // Some sobre todas as empilhadeiras 'e'
            expr += z[e][a];
        }
        modelo.add(expr == 1);
        expr.end();
    }

    // Expressão 16: Conecta o atendimento do talhão (z) com a rota da empilhadeira (y).
    // Garante que, se uma empilhadeira atende um talhão, um arco deve chegar e um arco deve sair.
    for (int e = 0; e < E; ++e) {
        for (int a = 0; a < T; ++a) { // Para cada empilhadeira 'e' e cada talhão 'a'
            IloExpr saida(env);
            IloExpr chegada(env);

            // 1ª parte: Soma dos arcos de SAÍDA do talhão 'a'
            // (para outros talhões 'b' ou para a garagem final T+1)
            for (int b = 0; b < T; ++b) {
                if (a != b) {
                    saida += y[e][a][b];
                }
            }
            saida += y[e][a][T + 1];

            // 2ª parte: Soma dos arcos de CHEGADA no talhão 'a'
            // (vindo de outros talhões 'b' ou da garagem inicial T)
            for (int b = 0; b < T; ++b) {
                if (a != b) {
                    chegada += y[e][b][a];
                }
            }
            chegada += y[e][T][a];

            // Adiciona as duas restrições ao modelo
            modelo.add(saida == z[e][a]);
            modelo.add(chegada == z[e][a]);

            saida.end();
            chegada.end();
        }
    }

    // Expressão 17: Eliminação de sub-rotas para empilhadeiras (contagem de arcos).
    // O número de arcos entre talhões é igual ao número de talhões servidos menos um.
    for (int e = 0; e < E; ++e) {
        // Lado Esquerdo: Soma das transições entre talhões distintos
        IloExpr transicoes(env);
        for (int a = 0; a < T; ++a) {
            for (int b = 0; b < T; ++b) {
                if (a != b) {
                    transicoes += y[e][a][b];
                }
            }
        }

        // Lado Direito: Soma dos talhões atendidos pela empilhadeira 'e'
        IloExpr talhoes_servidos(env);
        for (int a = 0; a < T; ++a) {
            talhoes_servidos += z[e][a];
        }

        // A restrição só se aplica se a empilhadeira for usada (atender >= 1 talhão),
        // para evitar a inviabilidade 0 = -1.
        modelo.add(IloIfThen(env, talhoes_servidos >= 1, transicoes == talhoes_servidos - 1));

        transicoes.end();
        talhoes_servidos.end();
    }

    // Expressão 18: Conservação de fluxo para cada talhão.
    // Para cada talhão 'b', o total de arcos que chegam deve ser igual ao total de arcos que saem.
    for (int e = 0; e < E; ++e) {
        for (int b = 0; b < T; ++b) { // Para cada talhão 'b'
            IloExpr total_chegadas(env);
            IloExpr total_saidas(env);
            
            // Somamos sobre todos os nós possíveis 'j'
            for (int j = 0; j < T + 2; ++j) {
                if (b != j) {
                    total_chegadas += y[e][j][b]; // Arcos que chegam em 'b' vindo de 'j'
                    total_saidas += y[e][b][j];   // Arcos que saem de 'b' para 'j'
                }
            }
            modelo.add(total_chegadas == total_saidas);
            total_chegadas.end();
            total_saidas.end();
        }
    }

    // Expressão 19: Cada empilhadeira 'e' deve sair da garagem de partida (T) exatamente uma vez.
    for (int e = 0; e < E; ++e) {
        IloExpr expr(env);
        // O destino pode ser qualquer talhão (0 a T-1) ou a garagem de chegada (T+1).
        for (int b = 0; b <= T + 1; ++b) {
            // A empilhadeira não pode ir da garagem de partida (T) para si mesma.
            if (b != T) {
                expr += y[e][T][b];
            }
        }
        modelo.add(expr == 1);
        expr.end();
    }

    // Expressão 20: Cada empilhadeira 'e' deve chegar na garagem de chegada (T+1) exatamente uma vez.
    for (int e = 0; e < E; ++e) {
        IloExpr expr(env);
        // A origem pode ser qualquer talhão (0 a T-1) ou a garagem de partida (T).
        for (int a = 0; a <= T + 1; ++a) {
            // A empilhadeira não pode chegar na garagem final (T+1) vindo dela mesma.
            if (a != T + 1) {
                expr += y[e][a][T + 1];
            }
        }
        modelo.add(expr == 1);
        expr.end();
    }

    // Expressão 21: Sequenciamento de tempo para as empilhadeiras (entre talhões).
    // Se uma empilhadeira vai de 'a' para 'b', o tempo de início em 'b' (c[e][b])
    // depende do tempo de início e da duração do serviço em 'a'.
    for (int e = 0; e < E; ++e) {
        for (int a = 0; a < T; ++a) {
            for (int b = 0; b < T; ++b) {
                if (a != b) {
                    // c_b >= c_a + (tempo de serviço em a) + (tempo de viagem a->b) - M*(1-y_ab)
                    // O tempo de serviço no talhão 'a' é o número de lotes (LT[a]) vezes o tempo C.
                    modelo.add(c[e][b] >= c[e][a] + (LT[a] * C) + DE[a][b] - M * (1 - y[e][a][b]));
                }
            }
        }
    }
    
    // Expressão 22: Define o tempo de início para o primeiro talhão visitado 
    // pela empilhadeira a partir da garagem.
    for (int e = 0; e < E; ++e) {
        for (int a = 0; a < T; ++a) { // Para cada talhão 'a'
            modelo.add(c[e][a] >= DE[T][a] * y[e][T][a]);
        }
    }

    //Resolução do matemático
    IloCplex cplex(modelo);
    cplex.exportModel("entrada_modelo.lp");
    cplex.solve();

    //Impressão dos resultados
    IloAlgorithm::Status status = cplex.getStatus();
    const char* statusString;
    switch (status) {
        case IloAlgorithm::Optimal:
            statusString = "Optimal";
            break;
        case IloAlgorithm::Infeasible:
            statusString = "Infeasible";
            break;
        case IloAlgorithm::Unbounded:
            statusString = "Unbounded";
            break;
        case IloAlgorithm::Feasible:
            statusString = "Feasible";
            break;
        default:
            statusString = "Unknown";
            break;
    }

    //Impressao dos resultados

    cout << "\n--------------------------------------------------" << endl;
    cout << "               RESULTADOS DA OTIMIZACAO" << endl;
    cout << "--------------------------------------------------" << endl;

    if (cplex.getStatus() == IloAlgorithm::Optimal || cplex.getStatus() == IloAlgorithm::Feasible) {
        
        const char* statusString = (cplex.getStatus() == IloAlgorithm::Optimal) ? "Otima Encontrada" : "Viavel Encontrada";
        printf("Status da solucao: %s\n", statusString);

        cout << "\n>> FUNCAO OBJETIVO (MAKESPAN)" << endl;
        printf("Tempo total para conclusao de todas as tarefas: %.2f horas\n", cplex.getValue(m));

        cout << "\n>> ROTAS DOS VEICULOS" << endl;
        for (int k = 0; k < V; k++) {
            IloNum totalLotesServidos = 0;
            for(int i = 0; i < L; i++) totalLotesServidos += cplex.getValue(s[k][i]);

            if (totalLotesServidos < 0.5) {
                printf("\nVeiculo %d: Nao utilizado.\n", k);
                continue;
            }

            printf("\n--- Veiculo %d ---\n", k);
            
            int loteAtual = -1;
            for (int j = 0; j < L; j++) {
                if (cplex.getValue(x[k][L][j]) > 0.9) {
                    loteAtual = j;
                    break;
                }
            }

            if(loteAtual == -1) {
                printf("Rota: Garagem -> Garagem\n");
                continue;
            }

            string rota = "Garagem -> ";
            // CORREÇÃO: Vetor para rastrear lotes já impressos nesta rota
            vector<bool> visitados(L, false); 
            int contadorSeguranca = 0; // Evita loop em caso de erro extremo

            while(loteAtual != -1 && contadorSeguranca < L + 1) {
                // Se já visitamos este lote na rota, encontramos um ciclo. Paramos.
                if(visitados[loteAtual]) {
                    rota += "... [CICLO DETECTADO]";
                    break;
                }
                visitados[loteAtual] = true;
                contadorSeguranca++;

                rota += "Lote " + to_string(loteAtual);

                printf("  - Lote %d:\n", loteAtual);
                printf("    - Chegada (b): %.2f\n", cplex.getValue(b[k][loteAtual]));
                printf("    - Inicio Carga (d): %.2f\n", cplex.getValue(d[k][loteAtual]));
                printf("    - Fim Carga (h): %.2f\n", cplex.getValue(h[loteAtual]));

                int proximoLote = -1;
                for (int j = 0; j < L; j++) {
                    if (loteAtual != j && cplex.getValue(x[k][loteAtual][j]) > 0.9) {
                        proximoLote = j;
                        break;
                    }
                }

                if (proximoLote == -1) {
                    rota += " -> Garagem";
                    loteAtual = -1;
                } else {
                    rota += " -> ";
                    loteAtual = proximoLote;
                }
            }
            cout << "\n  Sequencia da Rota: " << rota << endl;
        }

        cout << "\n\n>> ROTAS DAS EMPILHADEIRAS" << endl;
        for (int e = 0; e < E; e++) {
            IloNum totalTalhoesServidos = 0;
            for(int a = 0; a < T; a++) totalTalhoesServidos += cplex.getValue(z[e][a]);
             if (totalTalhoesServidos < 0.5) {
                printf("\nEmpilhadeira %d: Nao utilizada.\n", e);
                continue;
            }

            printf("\n--- Empilhadeira %d ---\n", e);

            int talhaoAtual = -1;
            for (int b = 0; b < T; b++) {
                if (cplex.getValue(y[e][T][b]) > 0.9) {
                    talhaoAtual = b;
                    break;
                }
            }

            if(talhaoAtual == -1) {
                 printf("Rota: Garagem -> Garagem\n");
                 continue;
            }

            string rota_emp = "Garagem -> ";
            vector<bool> visitados_emp(T, false);
            int contadorSeguranca_emp = 0;

            while(talhaoAtual != -1 && contadorSeguranca_emp < T + 1) {
                if(visitados_emp[talhaoAtual]){
                    rota_emp += "... [CICLO DETECTADO]";
                    break;
                }
                visitados_emp[talhaoAtual] = true;
                contadorSeguranca_emp++;
                
                rota_emp += "Talhao " + to_string(talhaoAtual);

                printf("  - Talhao %d:\n", talhaoAtual);
                printf("    - Inicio Servico (c): %.2f\n", cplex.getValue(c[e][talhaoAtual]));

                int proximoTalhao = -1;
                for (int b = 0; b < T; b++) {
                    if (talhaoAtual != b && cplex.getValue(y[e][talhaoAtual][b]) > 0.9) {
                        proximoTalhao = b;
                        break;
                    }
                }

                if (proximoTalhao == -1) {
                    rota_emp += " -> Garagem";
                    talhaoAtual = -1;
                } else {
                    rota_emp += " -> ";
                    talhaoAtual = proximoTalhao;
                }
            }
             cout << "\n  Sequencia da Rota: " << rota_emp << endl;
        }

    } else {
        cout << "\n>> NENHUMA SOLUCAO ENCONTRADA" << endl;
        cout << "Status da solucao: " << cplex.getStatus() << endl;
        cout << "Verifique o arquivo 'modelo.lp' para analisar as restricoes." << endl;
    }
    cout << "\n--------------------------------------------------" << endl;
    return 0;
}