# Optimisation with CPLEX
Risk-cost optimization using CPLEX library

## Problem
Power station has 42 electric power generators: 16 of the first type (T1), 14 of the second type (T2) and 12 of the third
type(T3). Generators have the following properties:

|  | min load | max load | hourly cost (min load) | hourly cost (min-max load) / MW | starting cost |
| --- | --- | --- | --- | --- | --- |
|  | MW | MW | $ | $ | $ |
| T1 | 1000 | 2000 | 1000 | R_1 | 2000 |
| T2 | 1300 | 1800 | 2500 | R_2 | 1500 |
| T3 | 1500 | 3000 | 3200 | R_3 | 1000 |

Cost per hour per MW between min and max load is determined with random vector R.
Vector R has 3 dimensional t-student distribution with 4 degrees of freedom. 

Power station has to fulfil following requirements:
* daily electricity demand:

| hour | electricity demand |
| --- | --- |
| midnight - 6am | 15000 |
| 6am - 9am| 35000 |
| 9am - 3pm | 20000 |
| 3pm - 6pm | 45000 |
| 6pm - midnight | 20000 |

* operating over 90% of max load cost additionally 200 $/hour
*power generators must be able to satisfy 10% increase in electricity demand 
(without operating beyond max load limit)

## Task 1 
Single-criterion selection model with the expected value as a measure of cost (folder `zadanie_1`).

## Task 2
Two-criterion cost and risk selection model with an average as a measure of cost and a standard deviation as a measure of risk
(folder `zadanie_2`).

Project was done as a part Decision Support under Risk Conditions Course at Warsaw University Technology.
