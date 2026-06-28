/*
 * Problema de Fluxo Maximo (PFM)
 * Variaveis inteiras, capacidade minima de cada arco = 0 (lij = 0).
 *
 * Modelo:
 *   max  saida(origem) - entrada(origem)
 *   s.a. saida(i) == entrada(i)         para todo no intermediario i
 *        0 <= x_ij <= capacidade_ij     (quando ha limite definido)
 *
 * Grafo direcionado (a capacidade so vale no sentido lido do in.txt).
 * Pares que nao sao arco do grafo tem o limite superior travado em 0.
 */

#include <bits/stdc++.h>
#include <ilcplex/ilocplex.h>

using namespace std;
ILOSTLBEGIN

#define TEMPO_LIMITE_CPLEX 3600

struct Arco {
    int capacidade = 0;
    bool existe = false;
};

int qtdNos;
int qtdArcos = 0;
vector<vector<Arco>> arcos;
int origem, destino;
char rotulo[] = "SABCDET";

void resolverPFM() {
    IloEnv env;
    int i, j, k;
    int totalVar = 0, totalRest = 0;

    IloModel model(env);

    IloArray<IloNumVarArray> x(env);
    for (i = 0; i < qtdNos; i++) {
        x.add(IloNumVarArray(env));
        for (j = 0; j < qtdNos; j++) {
            int limiteSuperior = arcos[i][j].existe ? CPXINT_MAX : 0;
            x[i].add(IloIntVar(env, 0, limiteSuperior));
            totalVar++;
        }
    }

    // objetivo: maximizar o fluxo que efetivamente sai da origem
    IloExpr saidaOrigem(env), entradaOrigem(env);
    for (j = 0; j < qtdNos; j++)
        if (arcos[origem][j].existe) saidaOrigem += x[origem][j];
    for (i = 0; i < qtdNos; i++)
        if (arcos[i][origem].existe) entradaOrigem += x[i][origem];
    model.add(IloMaximize(env, saidaOrigem - entradaOrigem));
    saidaOrigem.end();
    entradaOrigem.end();

    // conservacao de fluxo nos nos intermediarios
    for (i = 0; i < qtdNos; i++) {
        if (i == origem || i == destino) continue;

        IloExpr sai(env), entra(env);
        for (j = 0; j < qtdNos; j++)
            if (arcos[i][j].existe) sai += x[i][j];
        for (k = 0; k < qtdNos; k++)
            if (arcos[k][i].existe) entra += x[k][i];

        model.add(sai == entra);
        totalRest++;
        sai.end();
        entra.end();
    }

    // capacidade maxima dos arcos (quando informada)
    for (i = 0; i < qtdNos; i++) {
        for (j = 0; j < qtdNos; j++) {
            if (arcos[i][j].existe && arcos[i][j].capacidade > 0) {
                model.add(x[i][j] <= arcos[i][j].capacidade);
                totalRest++;
            }
        }
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
                double v = IloRound(cplex.getValue(x[i][j]));
                if (v != 0)
                    printf("x[%c][%c]: %.0f\n", rotulo[i], rotulo[j], v);
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

    int a, b, cap;
    while (cin >> a >> b >> cap) {
        arcos[a - 1][b - 1] = {cap, true};
        qtdArcos++;
    }

    cout << "Verificacao da leitura dos dados:\n";
    cout << "Num. de vertices: " << qtdNos << "\n";
    cout << "Quantidade de arcos: " << qtdArcos << "\n";
    cout << "No de origem: " << origem << "\n";
    cout << "No de destino: " << destino << "\n";
    for (int i = 0; i < qtdNos; i++)
        for (int j = 0; j < qtdNos; j++)
            if (arcos[i][j].existe)
                cout << "[" << rotulo[i] << "][" << rotulo[j]
                     << "] - Capacidade: " << arcos[i][j].capacidade << "\n";

    resolverPFM();
    return 0;
}
