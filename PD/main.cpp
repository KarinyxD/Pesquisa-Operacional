/*
 * Problema da Designacao (PD)
 * Variaveis binarias.
 *
 * Modelo:
 *   min  soma( custo_ij * x_ij )
 *   s.a. soma_j x_ij == 1   (cada pessoa recebe exatamente 1 tarefa)
 *        soma_i x_ij == 1   (cada tarefa recebe exatamente 1 pessoa)
 *        x_ij em {0,1}
 *
 * No in.txt as N/2 primeiras linhas representam pessoas e as N/2
 * seguintes representam tarefas (entrada quadrada, como o problema exige).
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
int qtdArcos;
vector<vector<Arco>> arcos;
int numPessoas; // = qtdNos / 2
char rotuloTarefa[] = "ABC";

void resolverPD() {
    IloEnv env;
    int i, j;
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

    // cada pessoa fica com exatamente uma tarefa
    for (i = 0; i < numPessoas; i++) {
        IloExpr soma(env);
        for (j = 0; j < qtdNos; j++)
            if (arcos[i][j].existe) soma += x[i][j];
        model.add(soma == 1);
        totalRest++;
        soma.end();
    }

    // cada tarefa fica com exatamente uma pessoa
    for (j = numPessoas; j < qtdNos; j++) {
        IloExpr soma(env);
        for (i = 0; i < qtdNos; i++)
            if (arcos[i][j].existe) soma += x[i][j];
        model.add(soma == 1);
        totalRest++;
        soma.end();
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
                    printf("x[%d][%c]: %.0f\n", i + 1, rotuloTarefa[j - numPessoas], v);
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
    cin >> qtdNos >> qtdArcos;
    numPessoas = qtdNos / 2;
    arcos.assign(qtdNos, vector<Arco>(qtdNos));

    for (int e = 0; e < qtdArcos; e++) {
        int a, b, custo;
        cin >> a >> b >> custo;
        arcos[a - 1][b - 1] = {custo, true};
    }

    cout << "Verificacao da leitura dos dados:\n";
    cout << "Num. de pessoas: " << numPessoas << "\n";
    cout << "Quantidade de pares pessoa-tarefa: " << qtdArcos << "\n";
    for (int i = 0; i < numPessoas; i++)
        for (int j = numPessoas; j < qtdNos; j++)
            if (arcos[i][j].existe)
                cout << "[" << i + 1 << "][" << rotuloTarefa[j - numPessoas]
                     << "] - Custo: " << arcos[i][j].custo << "\n";

    resolverPD();
    return 0;
}
