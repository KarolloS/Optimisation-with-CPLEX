#include <ilcplex/ilocplex.h>
#include <iostream>
#include <random>
ILOSTLBEGIN

default_random_engine generator_d; // deterministyczny
std::random_device rd;
default_random_engine generator_nd(rd()); // niedeterministyczny
student_t_distribution<double> distribution(4.0);

double get_t_student_random()
{
	double 	num = distribution(generator_d);

	if (num > 5 || num < 1)
		num = get_t_student_random();

	return num;
}

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
			data_generatory[3][k] = 0;
			data_generatory[4][k] = 2000.0;
			k++;
		}
		for (int j = 0; j < 14; j++)
		{
			data_generatory[0][k] = 1300.0;
			data_generatory[1][k] = 1800.0;
			data_generatory[2][k] = 2500.0;
			data_generatory[3][k] = 0;
			data_generatory[4][k] = 1500.0;
			k++;
		}
		for (int j = 0; j < 12; j++)
		{
			data_generatory[0][k] = 1500.0;
			data_generatory[1][k] = 3000.0;
			data_generatory[2][k] = 3200.0;
			data_generatory[3][k] = 0;
			data_generatory[4][k] = 1000.0;
			k++;
		}
	}

	// zapotrzebiwnia w kolejnych porach dnia
	double zapotrzebiwanie[5] = { 15000.0, 35000.0, 20000.0, 45000.0, 20000.0 };

	// dlugosc w godzinach kolejnych por dnia
	double pora[5] = { 6.0, 3.0, 6.0, 3.0, 6.0 };

	// odpoweiednia duza zmienna
	double M;

	// liczba scenariuszy
	const int s = 70;

	//liczba punktów
	int n = 100;

	// zapis do pliku
	ofstream F1;
	F1.open("dane_1.txt");
	ofstream F2;
	F2.open("dane_2.txt");

	// zmenne pomicnicze dla generacji wielu punktów
	double a_koszt = 0;
	double a_ryzyko = 0;

	// tablica zmiennych losowych dla wszytkich scenariuszy
	double vect_R[s][3];
	for (int i = 0; i < s; i++)
	{
		double p_1 = get_t_student_random();
		double p_3 = get_t_student_random();
		double p_2 = get_t_student_random();

		vect_R[i][1] = p_1;
		vect_R[i][2] = p_2;
		vect_R[i][3] = p_3;
	}


	for (int z = 0; z < n; z++)
	{
		IloEnv   env;
		try {
			IloModel model(env);
			IloArray<IloNumVarArray> var(env, 4);
			for (int i = 0; i < 4; i++)
			{
				var[i] = IloNumVarArray(env);
			}
			IloRangeArray con(env);


			// zmienne 0-209 -> czy dany generator jest aktualnie wlaczony
			for (int i = 0; i < 210; i++)
			{
				var[2].add(IloNumVar(env, 0, 1, ILOBOOL));
			}

			// zmienne 210-419 -> obciazenie powyzej obciazenia minimalnego
			for (int i = 0; i < 210; i++)
			{
				var[2].add(IloNumVar(env, 0, IloInfinity, ILOINT));
			}

			// zmienne 420-629 -> czy dany generator byl wlaczony w poprzedniej porze dnia i jest aktualnie wlaczony
			for (int i = 0; i < 210; i++)
			{
				var[2].add(IloNumVar(env, 0, 1, ILOBOOL));
			}

			// zmienne 630-839 -> czy dany generator pracuje powyzej 90% swojego maksymalnego obciazenia
			for (int i = 0; i < 210; i++)
			{
				var[2].add(IloNumVar(env, 0, 1, ILOBOOL));
			}

			/*******************************************************************************/

			// ograniczenie na maksymaln¹ wartosc obciazenia powyzej obciazenia minimalnego 
			// gorne ograniczenie 0 gdy generator nie dziala
			for (int i = 0; i < 210; i++)
			{
				con.add(var[2][i + 210] - ((data_generatory[1][i] - data_generatory[0][i]) * var[2][i]) <= 0);
			}

			// ogarniczenie na maksymalne zapotrzebowanie w kazdej porze dnia
			k = 0;
			for (int i = 0; i < 5; i++)
			{
				IloExpr expr(env);
				for (int j = 0; j < 42; j++)
				{
					//expr += ((data_generatory[1][k] - data_generatory[0][k]) * var[2][k]) + var[2][k + 210];
					expr += (data_generatory[0][k] * var[2][k]) + var[2][k + 210];
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
					con.add(var[2][i + 420] - var[2][i] == 0);
				}
				else
				{
					con.add(var[2][i + 420] - var[2][i] <= 0);
					con.add(var[2][i + 420] - var[2][i] + var[2][i - 42] >= 0);
					con.add(var[2][i + 420] - 2 + var[2][i] + var[2][i - 42] <= 0);
				}
			}

			// ogarniczenie dotyczace 10% zapasu w kazdej porze dnia
			k = 0;
			for (int i = 0; i < 5; i++)
			{
				IloExpr expr(env);
				for (int j = 0; j < 42; j++)
				{
					expr += (data_generatory[1][k] - data_generatory[0][k]) * var[2][k] - var[2][k + 210];
					k++;
				}
				con.add(expr >= zapotrzebiwanie[i] * 0.1);
				expr.end();
			}

			// okraniczenie akreslajace czy nalezy policzyc dodatkowy koszt za prace powyzej 90% maksymalnego obciazenia
			M = 10000;
			for (int i = 0; i < 210; i++)
			{
				con.add(var[2][i + 210] - (0.9*(data_generatory[1][i] - data_generatory[0][i]) * var[2][i]) - M * var[2][i + 630] <= 0);
				con.add(var[2][i + 210] - (0.9*(data_generatory[1][i] - data_generatory[0][i]) * var[2][i]) - M * (var[2][i + 630] - 1) >= 0);
			}


			/*******************************************************************************/

			//dodanie warunku minimalizacji kosztu - dla wielu scenariuszy
			for (int m = 0; m < s; m++)
			{
				// uaktualnienie kosztu godziny/MV powyzej minimalnego obciazenia na podstawie wektora losowego
				int k = 0;
				for (int i = 0; i < 5; i++)
				{
					for (int j = 0; j < 16; j++)
					{
						data_generatory[3][k] = vect_R[m][1];
						k++;
					}
					for (int j = 0; j < 14; j++)
					{
						data_generatory[3][k] = vect_R[m][2];
						k++;
					}
					for (int j = 0; j < 12; j++)
					{
						data_generatory[3][k] = vect_R[m][3];
						k++;
					}
				}

				// zmienna - ca³kowoty koszt dla wszystkich pór doby
				var[1].add(IloNumVar(env, 0, IloInfinity)); // indeks m

				IloExpr expr(env);
				k = 0;
				for (int i = 0; i < 5; i++)
				{
					for (int j = 0; j < 42; j++)
					{
						expr += var[2][k] * data_generatory[2][k];
						expr += var[2][k + 210] * data_generatory[3][k];
						expr += var[2][k + 630] * 200;
						k++;
					}
					expr = expr * pora[i];
				}

				k = 0;
				for (int i = 0; i < 5; i++)
				{
					for (int j = 0; j < 42; j++)
					{
						expr += var[2][k + 420] * data_generatory[4][k];
						k++;
					}
				}
				con.add(expr - var[1][m] == 0);
				expr.end();
			}

			/*******************************************************************************/

			// sredni koszt dla wszystkich scenariuszy
			var[0].add(IloNumVar(env, 0, IloInfinity)); // indeks 0

			IloExpr expr_1(env);
			for (int i = 0; i < s; i++)
			{
				expr_1 += var[1][i];
			}
			expr_1 = expr_1 / s;
			con.add(expr_1 - var[0][0] == 0);
			expr_1.end();



			// odchylenie przeciêtne dla wszystkich scenariuszy
			var[0].add(IloNumVar(env, 0, IloInfinity)); // indeks 1

			// zmienne 2-s+1 -> wartosc modulu w odchyleniu przzecietnym
			for (int i = 0; i < s; i++)
			{
				var[0].add(IloNumVar(env, 0, IloInfinity));
			}

			// zmienne s+2-2s+1 -> zmienna pomocnicza do obliczen modulu
			for (int i = 0; i < s; i++)
			{
				var[0].add(IloNumVar(env, 0, 1, ILOBOOL));
			}

			// obliczenie odchylenia przecietnego -> ryzyko
			M = 2.0e10;
			for (int i = 0; i < s; i++)
			{
				con.add((var[0][0] - var[1][i]) + M * var[0][i + s + 2] - var[0][i + 2] >= 0);
				con.add(-(var[0][0] - var[1][i]) + M * (1 - var[0][i + s + 2]) - var[0][i + 2] >= 0);
				con.add((var[0][0] - var[1][i]) - var[0][i + 2] <= 0);
				con.add(-(var[0][0] - var[1][i]) - var[0][i + 2] <= 0);
			}

			IloExpr expr_2(env);
			for (int i = 0; i < s; i++)
			{
				expr_2 += var[0][i + 2];
			}
			expr_2 = expr_2 / s;
			con.add(expr_2 - var[0][1] == 0);
			expr_2.end();


			//a_koszt = 5.0e7 + (5.15e7 - 5.0e7) / (n - 1) * z;
			//con.add(var[0][0] <= a_koszt);

			//model.add(con);
			//model.add(IloMinimize(env, var[0][1]));

			/*******************************************************************************/

			a_koszt = 5.0e7 + (5.15e7 - 5.0e7) / (n - 1) * z;
			a_ryzyko = 850000 - 850000 / (n - 1) * z;

			//metoda punktu odniesienia
			double epsilon = 0.001 / s;
			double beta = 0.001;

			// zmienna pomocnicza v
			var[3].add(IloNumVar(env, -IloInfinity, IloInfinity)); // indeks 0

			// zmienna pomocnicza z_1
			var[3].add(IloNumVar(env, -IloInfinity, IloInfinity)); // indeks 1
			// zmienna pomocnicza z_2
			var[3].add(IloNumVar(env, -IloInfinity, IloInfinity)); // indeks 2

			// ograniczenia		
			con.add(var[3][0] - var[3][1] <= 0);
			con.add(var[3][0] - var[3][2] <= 0);

			con.add(beta * (a_koszt - var[0][0]) - var[3][1] >= 0);
			con.add((a_koszt - var[0][0]) - var[3][1] >= 0);

			con.add(beta * (a_ryzyko - var[0][1]) - var[3][2] >= 0);
			con.add((a_ryzyko - var[0][1]) - var[3][2] >= 0);

			model.add(con);
			model.add(IloMaximize(env, var[3][0] + epsilon * (var[3][1] + var[3][2])));


			////metoda punktu odniesienia - uproszczona
			//double epsilon = 0.001 / s;

			//// zmienna pomocnicza v
			//var[3].add(IloNumVar(env, -IloInfinity, IloInfinity)); // indeks 0

			//// ograniczenia		
			//con.add((a_koszt - var[0][0]) - var[3][0] >= 0);
			//con.add((a_ryzyko - var[0][1]) - var[3][0] >= 0);

			//model.add(con);
			//model.add(IloMaximize(env, var[3][0] + epsilon * ((a_ryzyko - var[0][1]) + (a_koszt - var[0][0]))));


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

			double koszt = cplex.getValue(var[0][0]);
			double ryzyko = cplex.getValue(var[0][1]);

			cout << "sredni koszt: " << koszt << endl;
			cout << "odchylenie przecietne (ryzyko): " << ryzyko << endl << endl;

			F1 << z + 1 << "," << a_koszt << "," << a_ryzyko << "," << koszt << "," << ryzyko << "," << endl;

			double result[s];
			F2 << z + 1 << "," << endl;
			for (int i = 0; i < s; i++)
			{
				result[i] = cplex.getValue(var[1][i]);
				F2 << i << "," << result[i] << "," << endl;
				//cout << "Scenariusz " << i + 1 << " -> " << result[i] << endl;
			}
			//cout << endl;
			F2 << endl;

		}
		catch (IloException& e) {
			cerr << "Concert exception caught: " << e << endl;
		}
		catch (...) {
			cerr << "Unknown exception caught" << endl;
		}

		env.end();
	}

	F1.close();
	F2.close();
	system("pause");
	return 0;
}