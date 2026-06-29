/*
 * Problema do Caminho Minimo (PCM)
 * Variaveis binarias.
 *
 * Modelo:
 *   min  soma( custo_ij * x_ij )
 *   s.a. saida do no origem  == 1
 *        chegada no no destino == 1
 *        saida(k) == entrada(k)   para todo no intermediario k
 *        x_ij em {0,1}
 *
 * Grafo nao-direcionado: ao ler uma aresta (a,b,custo) o arco e
 * espelhado nos dois sentidos.
 *
 * Importante: x_ij so existe de fato quando (i,j) e uma aresta do
 * grafo. Pares que nao sao aresta tem o limite superior da variavel
 * travado em 0, para nao aparecerem "caminhos fantasmas" na solucao.
 */

#include <bits/stdc++.h>
#include <ilcplex/ilocplex.h>

using namespace std;
ILOSTLBEGIN

#define TEMPO_LIMITE_CPLEX 3600

struct Arco {
    int custo = 0;
    bool existe = false;
};

int qtdNos;
int qtdArcos = 0;
vector<vector<Arco>> arcos;
int origem, destino;

void resolverPCM() {
    IloEnv env;
    int i, j, k;
    int totalVar = 0, totalRest = 0;

    IloModel model(env);

    IloArray<IloNumVarArray> x(env);
    for (i = 0; i < qtdNos; i++) {
        x.add(IloNumVarArray(env));
        for (j = 0; j < qtdNos; j++) {
            int limiteSuperior = arcos[i][j].existe ? 1 : 0;
            x[i].add(IloIntVar(env, 0, limiteSuperior));
            totalVar++;
        }
    }

    IloExpr fo(env);
    for (i = 0; i < qtdNos; i++)
        for (j = 0; j < qtdNos; j++)
            if (arcos[i][j].existe)
                fo += arcos[i][j].custo * x[i][j];
    model.add(IloMinimize(env, fo));
    fo.end();

    // sai exatamente um arco da origem
    IloExpr saidaOrigem(env);
    for (j = 0; j < qtdNos; j++)
        if (arcos[origem][j].existe) saidaOrigem += x[origem][j];
    model.add(saidaOrigem == 1);
    totalRest++;
    saidaOrigem.end();

    // chega exatamente um arco no destino
    IloExpr entradaDestino(env);
    for (i = 0; i < qtdNos; i++)
        if (arcos[i][destino].existe) entradaDestino += x[i][destino];
    model.add(entradaDestino == 1);
    totalRest++;
    entradaDestino.end();

    // conservacao de fluxo nos nos intermediarios
    for (k = 0; k < qtdNos; k++) {
        if (k == origem || k == destino) continue;

        IloExpr entra(env), sai(env);
        for (i = 0; i < qtdNos; i++)
            if (arcos[i][k].existe) entra += x[i][k];
        for (j = 0; j < qtdNos; j++)
            if (arcos[k][j].existe) sai += x[k][j];

        model.add(entra == sai);
        totalRest++;
        entra.end();
        sai.end();
    }

    cout << "--------Informacoes da Execucao:----------\n\n";
    cout << "#Var: " << totalVar << "\n";
    cout << "#Restricoes: " << totalRest << "\n";

    IloCplex cplex(model);
    cplex.setParam(IloCplex::TiLim, TEMPO_LIMITE_CPLEX);

    time_t t1, t2;
    time(&t1);
    cplex.solve();
    time(&t2);

    string status;
    bool achouSolucao = true;
    switch (cplex.getStatus()) {
        case IloAlgorithm::Optimal:  status = "Optimal";  break;
        case IloAlgorithm::Feasible: status = "Feasible"; break;
        default:
            status = "No Solution";
            achouSolucao = false;
    }

    cout << "\nStatus da FO: " << status << "\n";

    if (achouSolucao) {
        cout << "Variaveis de decisao: \n";
        for (i = 0; i < qtdNos; i++) {
            for (j = 0; j < qtdNos; j++) {
                if (!arcos[i][j].existe) continue; // variavel nunca entrou no modelo, nem pergunta
                double v = IloRound(cplex.getValue(x[i][j]));
                if (v != 0)
                    printf("x[%c][%c]: %.0f\n", i + 'A', j + 'A', v);
            }
        }
        cout << "\nFuncao Objetivo Valor = " << cplex.getObjValue() << "\n";
        printf("..(%.6f seconds).\n\n", difftime(t2, t1));
    } else {
        cout << "No Solution!\n";
    }

    cplex.end();
    env.end();
}

int main() {
    cin >> qtdNos;
    arcos.assign(qtdNos, vector<Arco>(qtdNos));

    cin >> origem >> destino;
    origem -= 1;
    destino -= 1;

    int a, b, custo;
    while (cin >> a >> b >> custo) {
        arcos[a - 1][b - 1] = {custo, true};
        arcos[b - 1][a - 1] = {custo, true}; // grafo nao-direcionado
        qtdArcos++;
    }

    cout << "Verificacao da leitura dos dados:\n";
    cout << "Num. de nos: " << qtdNos << "\n";
    cout << "Quantidade de arcos: " << qtdArcos << "\n";
    cout << "No de origem: " << origem << "\n";
    cout << "No de destino: " << destino << "\n";
    for (int i = 0; i < qtdNos; i++)
        for (int j = 0; j < qtdNos; j++)
            if (arcos[i][j].existe)
                cout << "[" << (char)(i + 'A') << "][" << (char)(j + 'A')
                     << "] - Custo: " << arcos[i][j].custo << "\n";

    resolverPCM();
    return 0;
}
