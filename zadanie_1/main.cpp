#include <ilcplex/ilocplex.h>
ILOSTLBEGIN

int main()
{
	// obciazenie minimalne
	// obciazenie maksymalne
	// koszt godziny przy minimalnym obciazeniu
	// koszt godziny/MV powyzej minimalnego obciazenia
	// koszt uruchomienia
	double data_generatory[5][210];
	int k = 0;
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			data_generatory[0][k] = 1000.0;
			data_generatory[1][k] = 2000.0;
			data_generatory[2][k] = 1000.0;
			data_generatory[3][k] = 2.6555;
			data_generatory[4][k] = 2000.0;
			k++;
		}
		for (int j = 0; j < 14; j++)
		{
			data_generatory[0][k] = 1300.0;
			data_generatory[1][k] = 1800.0;
			data_generatory[2][k] = 2500.0;
			data_generatory[3][k] = 2.9068;
			data_generatory[4][k] = 1500.0;
			k++;
		}
		for (int j = 0; j < 12; j++)
		{
			data_generatory[0][k] = 1500.0;
			data_generatory[1][k] = 3000.0;
			data_generatory[2][k] = 3200.0;
			data_generatory[3][k] = 3.0806;
			data_generatory[4][k] = 1000.0;
			k++;
		}
	}

	// zapotrzebiwnia w kolejnych porach dnia
	double zapotrzebiwanie[5] = { 15000.0, 35000.0, 20000.0, 45000.0, 20000.0 };

	// dlugosc w godzinach kolejnych por dnia
	double pora[5] = { 6.0, 3.0, 6.0, 3.0, 6.0 };


	IloEnv   env;
	try {
		IloModel model(env);
		IloNumVarArray var(env);
		IloRangeArray con(env);

		// zmienne 0-209 -> czy dany generator jest aktualnie wlaczony
		for (int i = 0; i < 210; i++)
		{
			var.add(IloNumVar(env, 0, 1, ILOBOOL));
		}

		// zmienne 210-419 -> obciazenie powyzej obciazenia minimalnego
		for (int i = 0; i < 210; i++)
		{
			var.add(IloNumVar(env, 0, IloInfinity, ILOINT));
		}

		// zmienne 420-629 -> czy dany generator byl wlaczony w poprzedniej porze dnia i jest aktualnie wlaczony
		for (int i = 0; i < 210; i++)
		{
			var.add(IloNumVar(env, 0, 1, ILOBOOL));
		}

		// zmienne 630-839 -> czy dany generator pracuje powyzej 90% swojego maksymalnego obciazenia
		for (int i = 0; i < 210; i++)
		{
			var.add(IloNumVar(env, 0, 1, ILOBOOL));
		}

		// zmienna 840 ca³kowoty koszt dla wszystkich pór doby
		var.add(IloNumVar(env, 0, IloInfinity));

		/*******************************************************************************/

		// ograniczenie na maksymaln¹ wartosc obciazenia powyzej obciazenia minimalnego 
		// gorne ograniczenie 0 gdy generator nie dziala
		for (int i = 0; i < 210; i++)
		{
			con.add(var[i + 210] - ((data_generatory[1][i] - data_generatory[0][i]) * var[i]) <= 0);
		}

		// ogarniczenie na maksymalne zapotrzebowanie w kazdej porze dnia
		k = 0;
		for (int i = 0; i < 5; i++)
		{
			IloExpr expr(env);
			for (int j = 0; j < 42; j++)
			{
				expr += (data_generatory[0][k] * var[k]) + var[k + 210];
				k++;
			}
			con.add(expr >= zapotrzebiwanie[i]);
			expr.end();
		}

		// ogarniczenia okreslajace czy nalezy policzyc koszt uruchomienia generatora
		for (int i = 0; i < 210; i++)
		{
			if (i < 42)
			{
				con.add(var[i + 420] - var[i] == 0);
			}
			else
			{
				con.add(var[i + 420] - var[i] <= 0);
				con.add(var[i + 420] - var[i] + var[i - 42] >= 0);
				con.add(var[i + 420] - 2 + var[i] + var[i - 42] <= 0);
			}
		}

		// ogarniczenie dotyczace 10% zapasu w kazdej porze dnia
		k = 0;
		for (int i = 0; i < 5; i++)
		{
			IloExpr expr(env);
			for (int j = 0; j < 42; j++)
			{
				expr += (data_generatory[1][k] - data_generatory[0][k]) * var[k] - var[k + 210];
				k++;
			}
			con.add(expr >= zapotrzebiwanie[i] * 0.1);
			expr.end();
		}

		// okraniczenie akreslajace czy nalezy policzyc dodatkowy koszt za prace powyzej 90% maksymalnego obciazenia
		int M = 10000;
		for (int i = 0; i < 210; i++)
		{
			con.add(var[i + 210] - (0.9*(data_generatory[1][i] - data_generatory[0][i]) * var[i]) - M * var[i + 630] <= 0);
			con.add(var[i + 210] - (0.9*(data_generatory[1][i] - data_generatory[0][i]) * var[i]) - M * (var[i + 630] - 1) >= 0);
		}


		/*******************************************************************************/

		//dodanie warunku minimalizacji kosztu - zmienna var[840]
		IloExpr expr(env);
		k = 0;
		for (int i = 0; i < 5; i++)
		{
			for (int j = 0; j < 42; j++)
			{
				expr += var[k] * data_generatory[2][k] * pora[i];
				expr += var[k + 210] * data_generatory[3][k] * pora[i];
				expr += var[k + 630] * 200 * pora[i];
				k++;
			}
			//expr = expr * pora[i];
		}

		k = 0;
		for (int i = 0; i < 5; i++)
		{
			for (int j = 0; j < 42; j++)
			{
				expr += var[k + 420] * data_generatory[4][k];
				k++;
			}
		}
		con.add(expr - var[840] == 0);
		expr.end();

		/*******************************************************************************/

		model.add(con);

		// minimalizacja kosztu
		model.add(IloMinimize(env, var[840]));

		/*******************************************************************************/

		IloCplex cplex(model);
		//cplex.exportModel("lpex1.lp");

		// Optimizer time limit in seconds
		cplex.setParam(IloCplex::Param::TimeLimit, 180);

		// Optimize the problem and obtain solution.
		if (!cplex.solve()) {
			env.error() << "Failed to optimize LP" << endl;
			throw(-1);
		}

		IloNumArray vals(env);
		env.out() << "Solution status = " << cplex.getStatus() << endl;
		env.out() << "Solution value  = " << cplex.getObjValue() << endl;


		// drukowanie wynikow
		cout << endl << "________________________________________________________" << endl << endl;

		double result[840];
		for (int i = 0; i < 840; i++)
		{
			result[i] = cplex.getValue(var[i]);
		}

		k = 0;
		for (int i = 0; i < 5; i++)
		{
			cout << "Pora dnia numer: " << (i + 1) << endl << endl;
			for (int j = 0; j < 16; j++)
			{
				cout << "Gnerator T1." << (j + 1) << "\t" << result[k] << "\t" << result[k + 210] << "\t" << result[k + 420] << "\t" << result[k + 630] << endl;
				k++;
			}
			cout << endl;

			for (int j = 0; j < 14; j++)
			{
				cout << "Gnerator T2." << (j + 1) << "\t" << result[k] << "\t" << result[k + 210] << "\t" << result[k + 420] << "\t" << result[k + 630] << endl;
				k++;
			}
			cout << endl;

			for (int j = 0; j < 12; j++)
			{
				cout << "Gnerator T3." << (j + 1) << "\t" << result[k] << "\t" << result[k + 210] << "\t" << result[k + 420] << "\t" << result[k + 630] << endl;
				k++;
			}
			cout << endl;
		}

		//cplex.getValues(vals, var);
		//env.out() << "Values = " << endl << vals << endl;
		//cplex.getSlacks(vals, con);
		//env.out() << "Slacks        = " << vals << endl;
		//cplex.getDuals(vals, con);
		//env.out() << "Duals         = " << vals << endl;
		//cplex.getReducedCosts(vals, var);
		//env.out() << "Reduced Costs = " << vals << endl;

	}
	catch (IloException& e) {
		cerr << "Concert exception caught: " << e << endl;
	}
	catch (...) {
		cerr << "Unknown exception caught" << endl;
	}

	env.end();

	system("pause");
	return 0;
}