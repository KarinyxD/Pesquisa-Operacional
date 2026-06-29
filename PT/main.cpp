/*
 * Problema do Transporte (PT)
 * Variaveis inteiras.
 *
 * Modelo:
 *   min  soma( custo_ij * x_ij )
 *   s.a. soma_j x_ij <= oferta_i      (para cada origem i)
 *        soma_i x_ij == demanda_j     (para cada destino j)
 *        x_ij >= 0 e inteiro
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

struct No {
    int oferta = 0;
    int demanda = 0;
};

int qtdNos;
int qtdArcos;
vector<vector<Arco>> arcos;
vector<No> nos;

void resolverPT() {
    IloEnv env;
    int i, j;
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

    IloExpr fo(env);
    for (i = 0; i < qtdNos; i++)
        for (j = 0; j < qtdNos; j++)
            if (arcos[i][j].existe)
                fo += arcos[i][j].custo * x[i][j];
    model.add(IloMinimize(env, fo));
    fo.end();

    // cada origem nao pode enviar mais do que a sua oferta
    for (i = 0; i < qtdNos; i++) {
        if (nos[i].oferta > 0) {
            IloExpr saida(env);
            for (j = 0; j < qtdNos; j++)
                if (arcos[i][j].existe) saida += x[i][j];
            model.add(saida <= nos[i].oferta);
            totalRest++;
            saida.end();
        }
    }

    // cada destino precisa receber exatamente a sua demanda
    for (j = 0; j < qtdNos; j++) {
        if (nos[j].demanda > 0) {
            IloExpr entrada(env);
            for (i = 0; i < qtdNos; i++)
                if (arcos[i][j].existe) entrada += x[i][j];
            model.add(entrada == nos[j].demanda);
            totalRest++;
            entrada.end();
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
                if (!arcos[i][j].existe) continue; // variavel nunca entrou no modelo, nem pergunta
                double v = IloRound(cplex.getValue(x[i][j]));
                if (v != 0)
                    printf("x[%d][%d]: %.0f\n", i + 1, j - 2, v);
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
    nos.resize(qtdNos);
    arcos.assign(qtdNos, vector<Arco>(qtdNos));

    for (int i = 0; i < qtdNos; i++)
        cin >> nos[i].oferta >> nos[i].demanda;

    for (int e = 0; e < qtdArcos; e++) {
        int a, b, custo;
        cin >> a >> b >> custo;
        arcos[a - 1][b - 1] = {custo, true};
    }

    cout << "Verificacao da leitura dos dados:\n";
    cout << "Num. origens/destinos: " << qtdNos << "\n";
    cout << "Quantidade de pares origem-destino: " << qtdArcos << "\n";
    cout << "Nos: [oferta] [demanda]\n";
    for (int i = 0; i < qtdNos; i++)
        cout << "[" << nos[i].oferta << "][" << nos[i].demanda << "]\n";
    cout << "\n";

    resolverPT();
    return 0;
}
