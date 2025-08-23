#include<iostream>
#include<stdio.h>
#include<sstream>
#include <ilcplex/ilocplex.h>

typedef IloArray<IloNumVarArray> IloNumVarMatrix;
typedef IloArray<IloNumVarMatrix> IloNumVar3Matrix;
typedef IloArray<IloNumVar3Matrix> IloNumVar4Matrix;

using namespace std;

int main(int argc, char *argv[]) { 
    // --- LEITURA DE DADOS (AJUSTADA PARA ÍNDICE BASE-1) ---
    IloEnv env;

    int L_count, V_count, T_count, E_count;
    float C_param;

    FILE* fp = fopen(argv[1],"r");
    if( fp == NULL) {
        printf("Erro ao abrir o arquivo!\n");
        return 1;
    }
    
    fscanf(fp, "%d %d %d %d %f", &L_count, &V_count, &T_count, &E_count, &C_param);

    // CORREÇÃO: Usando IloArrays que são mais seguros e integrados com o CPLEX
    IloIntArray LT(env, T_count + 1);
    IloArray<IloIntArray> LE(env, L_count + 1);
    for(int i=0; i<=L_count; ++i) LE[i] = IloIntArray(env, T_count + 1);
    
    IloNumArray Ti(env, L_count + 1);
    IloNumArray Tv(env, L_count + 1);
    IloArray<IloNumArray> DE(env, T_count + 1);
    for(int i=0; i<=T_count; ++i) DE[i] = IloNumArray(env, T_count + 1);

    // Leitura LT
    for (int i=1; i<=T_count; i++){
        fscanf(fp, "%d", &LT[i]);
    }

    // Leitura LE
    for (int i=1; i<=L_count; i++){
        for (int j=1; j<=T_count; j++) {
            fscanf(fp, "%d", &LE[i][j]);
        }
    }

    // Leitura Ti
    for (int i=1; i<=L_count; i++) {
        fscanf(fp, "%f", &Ti[i]);
    }

    // Leitura Tv
    for (int i=1; i<=L_count; i++) {
        fscanf(fp, "%f", &Tv[i]);
    }

    // Leitura DE
    for (int i=1; i<=T_count; i++) {
        for (int j=1; j<=T_count; j++) {  
            fscanf(fp, "%f", &DE[i][j]);
        }    
    }
    fclose(fp);

    // --- DECLARAÇÃO DO MODELO ---
	IloModel modelo(env);
    
    // --- DECLARAÇÃO DAS VARIÁVEIS DE DECISÃO ---
    // CORREÇÃO: Renomeando variáveis para clareza e usando constantes
    const int L = L_count;
    const int V = V_count;
    const int T = T_count;
    const int E = E_count;

    // x[k][i][j]: veículo k vai do nó i para o nó j
    IloNumVar3Matrix x(env, V);
    for (int k = 0; k < V; ++k) {
        x[k] = IloNumVarMatrix(env, L + 2);
        for (int i = 0; i < L + 2; ++i) {
            x[k][i] = IloNumVarArray(env, L + 2, 0, 1, ILOBOOL);
        }
    }

    // s[k][i]: veículo k atende o lote i
    // CORREÇÃO: Dimensão L+1 para representar lotes de 1 a L
    IloNumVarMatrix s(env, V);
    for (int k = 0; k < V; ++k) {
        s[k] = IloNumVarArray(env, L + 1, 0, 1, ILOBOOL);
    }

    // b[k][i]: instante de chegada do veículo k no lote i
    // CORREÇÃO: Dimensão L+1
    IloNumVarMatrix b(env, V);
    for (int k = 0; k < V; ++k) {
        b[k] = IloNumVarArray(env, L + 1, 0, IloInfinity, ILOFLOAT);
    }
    
    // d[k][i]: instante de início de carregamento do veículo k no lote i
    // CORREÇÃO: Dimensão L+1
    IloNumVarMatrix d(env, V);
    for (int k = 0; k < V; ++k) {
        d[k] = IloNumVarArray(env, L + 1, 0, IloInfinity, ILOFLOAT);
    }
    
    // w[k][i]: tempo de espera do veículo k no lote i
    // CORREÇÃO: Dimensão L+1
    IloNumVarMatrix w(env, V);
    for (int k = 0; k < V; ++k) {
        w[k] = IloNumVarArray(env, L + 1, 0, IloInfinity, ILOFLOAT);
    }    
    
    // h[i]: instante de atendimento (fim do carregamento) do lote i
    // CORREÇÃO: Dimensão L+1
    IloNumVarArray h(env, L + 1, 0, IloInfinity, ILOFLOAT);

    // y[e][a][b]: empilhadeira e vai do talhão a para o b
    IloNumVar3Matrix y(env, E);
    for (int e = 0; e < E; ++e) {
        y[e] = IloNumVarMatrix(env, T + 2);
        for (int a = 0; a < T + 2; ++a) {
            y[e][a] = IloNumVarArray(env, T + 2, 0, 1, ILOBOOL);
        }
    }

    // z[e][a]: empilhadeira e atende o talhão a
    // CORREÇÃO: Dimensão T+1
    IloNumVarMatrix z(env, E);
    for (int e = 0; e < E; ++e) {
        z[e] = IloNumVarArray(env, T + 1, 0, 1, ILOBOOL);
    }
    
    // c[e][a]: instante de início de atendimento da empilhadeira e no talhão a
    // CORREÇÃO: Dimensão T+1
    IloNumVarMatrix c(env, E);
    for (int e = 0; e < E; ++e) {
        c[e] = IloNumVarArray(env, T + 1, 0, IloInfinity, ILOFLOAT);
    }
    
    IloNumVar m(env, 0, IloInfinity, ILOFLOAT, "makespan");
    
    // --- FUNÇÃO OBJETIVO ---
    modelo.add(IloMinimize(env, m));

    // --- RESTRIÇÕES ---
    // Expressão 2: O makespan (m) deve ser maior ou igual ao tempo de conclusão de cada lote.
    // CORREÇÃO: Loop de 1 a L, conforme a definição ∀ i ∈ L
    for (int i = 1; i <= L; ++i) {
        modelo.add(h[i] <= m);
    }
    
    // Expressão 3: Cada lote deve ser atendido por exatamente um veículo.
    // CORREÇÃO: Loop de 1 a L.
    for (int i = 1; i <= L; ++i) {
        IloExpr expr(env);
        for (int k = 0; k < V; ++k) {
            expr += s[k][i];
        }
        modelo.add(expr == 1);
        expr.end();
    }

    // Expressão 4 (reformulada): Se um veículo k visita um lote i, um arco deve entrar e um deve sair (soma=2). Se não visita, nenhum arco entra ou sai (soma=0).
    // CORREÇÃO: Loops de i e j devem percorrer os lotes reais (1 a L).
    for (int k = 0; k < V; ++k) {
        for (int i = 1; i <= L; ++i) {
            IloExpr soma_arcos(env);
            for (int j = 1; j <= L; ++j) {
                if (i != j) {
                    soma_arcos += x[k][i][j];
                    soma_arcos += x[k][j][i];
                }
            }
            // CORREÇÃO: Esta restrição não é exatamente a (4), mas a (9) junto com outras garantem a mesma coisa. Uma forma comum é linkar x com s:
            // Para cada j, x_ij <= s_i. Ou seja, se s_i é 0, nenhum arco pode sair de i.
            // A forma mais forte é a de conservação de fluxo (Expressão 9), implementada mais abaixo.
            // Para ser fiel à Expressão 4, podemos fazer:
            IloExpr saida(env), chegada(env);
             for (int j = 0; j < L+2; ++j) {
                if (i != j) {
                    saida += x[k][i][j];
                    chegada += x[k][j][i];
                }
            }
            modelo.add(saida == s[k][i]);
            modelo.add(chegada == s[k][i]);
            saida.end();
            chegada.end();
        }
    }
    
    // Expressão 5: REMOVIDA. A combinação das restrições 7, 8 e 9 garante a mesma lógica de forma mais robusta e padrão.

    // Expressão 6: Número de arcos ENTRE LOTES é igual ao número de lotes servidos menos 1.
    // CORREÇÃO: Usando IloIfThen para aplicar a restrição apenas se o veículo for usado.
    for (int k = 0; k < V; ++k) {
        IloExpr transicoes(env);
        for (int i = 1; i <= L; ++i) {
            for (int j = 1; j <= L; ++j) {
                if (i != j) {
                    transicoes += x[k][i][j];
                }
            }
        }
        IloExpr lotes_servidos(env);
        for (int i = 1; i <= L; ++i) {
            lotes_servidos += s[k][i];
        }
        modelo.add(IloIfThen(env, lotes_servidos >= 1, transicoes == lotes_servidos - 1));
        transicoes.end();
        lotes_servidos.end();
    }

    // Expressão 7: Cada veículo deve sair do depósito (nó 0) exatamente uma vez.
    // CORREÇÃO: Implementação correta. O destino j pode ser qualquer lote (1..L) ou o depósito final (L+1).
    for (int k = 0; k < V; ++k) {
        IloExpr expr(env);
        for (int j = 1; j <= L + 1; ++j) {
            expr += x[k][0][j];
        }
        modelo.add(expr == 1);
        expr.end();
    }

    // Expressão 8: Cada veículo deve chegar ao depósito final (nó L+1) exatamente uma vez.
    // CORREÇÃO: Implementação correta. A origem i pode ser o depósito inicial (0) ou qualquer lote (1..L).
    for (int k = 0; k < V; ++k) {
        IloExpr expr(env);
        for (int i = 0; i <= L; ++i) {
            expr += x[k][i][L + 1];
        }
        modelo.add(expr == 1);
        expr.end();
    }

    // Expressão 9: Continuidade de fluxo para os lotes.
    // CORREÇÃO: Para cada lote i, o número de arcos que entram deve ser igual ao número de arcos que saem.
    for (int k = 0; k < V; ++k) {
        for (int i = 1; i <= L; ++i) {
            IloExpr chegada(env);
            IloExpr saida(env);
            for (int j = 0; j <= L + 1; ++j) {
                if (i != j) {
                    if (j <= L) chegada += x[k][j][i]; // j pode ser 0 ou outro lote
                    if (j > 0) saida += x[k][i][j];   // j pode ser outro lote ou L+1
                }
            }
            modelo.add(chegada - saida == 0);
            chegada.end();
            saida.end();
        }
    }
    
    // ... (demais restrições 10 em diante, que não estavam no seu trecho, devem seguir a mesma lógica de correção de índices)

    // --- RESOLUÇÃO DO MODELO ---
    IloCplex cplex(modelo);
    cplex.exportModel("modelo_corrigido.lp");
    
    cout << "\nResolvendo o modelo..." << endl;
    if (cplex.solve()) {
        cout << "Status da solucao: " << cplex.getStatus() << endl;
        cout << "Valor da Funcao Objetivo (Makespan): " << cplex.getObjValue() << " horas" << endl;
    } else {
        cout << "Nao foi possivel encontrar uma solucao otima." << endl;
        cout << "Status: " << cplex.getStatus() << endl;
    }

    env.end();
    return 0;
}