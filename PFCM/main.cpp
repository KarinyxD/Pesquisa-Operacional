/*
 * Problema de Fluxo de Custo Minimo (PFCM)
 * Variaveis inteiras, capacidade minima de cada arco = 0 (lij = 0).
 *
 * Modelo:
 *   min  soma( custo_ij * f_ij )
 *   s.a. saida(i) - entrada(i) = oferta_i - demanda_i   para todo no i
 *        0 <= f_ij <= capacidade_ij   (quando ha limite definido)
 *
 * Observacao sobre os dados: quando a capacidade lida no in.txt for 0,
 * isso significa que o arco nao tem limite superior definido no grafo
 * (ou seja, e tratado como "sem restricao"), e nao capacidade nula.
 */

#include <bits/stdc++.h>
#include <ilcplex/ilocplex.h>

using namespace std;
ILOSTLBEGIN

#define TEMPO_LIMITE_CPLEX 3600

struct Arco {
    int custo = 0;
    int capacidade = 0;
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
char rotulo[] = "FGHMNPQRS"; // rotulos usados so para exibir a solucao

void resolverPFCM() {
    IloEnv env;
    int i, j;
    int totalVar = 0, totalRest = 0;

    IloModel model(env);

    // variaveis de fluxo f_ij; arcos inexistentes ficam travados em 0
    IloArray<IloNumVarArray> f(env);
    for (i = 0; i < qtdNos; i++) {
        f.add(IloNumVarArray(env));
        for (j = 0; j < qtdNos; j++) {
            int limiteSuperior = arcos[i][j].existe ? CPXINT_MAX : 0;
            f[i].add(IloIntVar(env, 0, limiteSuperior));
            totalVar++;
        }
    }

    // funcao objetivo: minimizar custo total transportado
    IloExpr fo(env);
    for (i = 0; i < qtdNos; i++)
        for (j = 0; j < qtdNos; j++)
            if (arcos[i][j].existe)
                fo += arcos[i][j].custo * f[i][j];
    model.add(IloMinimize(env, fo));
    fo.end();

    // balanco de fluxo em cada no: saida - entrada = oferta - demanda
    // (em nos de transbordo oferta = demanda = 0, logo fica saida = entrada)
    for (i = 0; i < qtdNos; i++) {
        IloExpr saida(env), entrada(env);
        for (j = 0; j < qtdNos; j++)
            if (arcos[i][j].existe) saida += f[i][j];
        for (j = 0; j < qtdNos; j++)
            if (arcos[j][i].existe) entrada += f[j][i];

        model.add(saida - entrada == nos[i].oferta - nos[i].demanda);
        totalRest++;
        saida.end();
        entrada.end();
    }

    // capacidade maxima dos arcos (quando informada)
    for (i = 0; i < qtdNos; i++) {
        for (j = 0; j < qtdNos; j++) {
            if (arcos[i][j].existe && arcos[i][j].capacidade > 0) {
                model.add(f[i][j] <= arcos[i][j].capacidade);
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
                double v = IloRound(cplex.getValue(f[i][j]));
                if (v != 0)
                    printf("f[%c][%c]: %.0f\n", rotulo[i], rotulo[j], v);
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
        int a, b, custo, cap;
        cin >> a >> b >> custo >> cap;
        arcos[a - 1][b - 1] = {custo, cap, true};
    }

    cout << "Verificacao da leitura dos dados:\n";
    cout << "Num. de nos: " << qtdNos << "\n";
    cout << "Qtd. de arcos: " << qtdArcos << "\n";
    cout << "Nos [oferta] [demanda]\n";
    for (int i = 0; i < qtdNos; i++)
        cout << "[" << nos[i].oferta << "][" << nos[i].demanda << "]\n";
    cout << "\n";

    resolverPFCM();
    return 0;
}
