#include "set_parser/SetParser.h"
#include "common/TaioData.h"
#include "common/TaioHashMap.h"
#include "common/TaioSetList.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAXPATH 25

int MetricOne(HashMap*);
int MetricTwo(HashMap*);
void PrepareLists(TaioSetList*, TaioSetList*, HashMap*);
double IterateSets(TaioSetList*, TaioSetList*);
double J(TaioSet*, TaioSet*);

TaioSet* FamilyToSet(TaioSetFamily*);
int SetToBin(TaioSet*);

double MetricThree(HashMap*, TaioData*);
double MetricThreeApprox(HashMap*, TaioData*);
int CountOnes(int);

double AlternativeMetric(TaioData*);

int main (int argc, char *argv[]){
    TaioData *parsedData, *reducedData;
    HashMap *map;             
    clock_t start, end;
    float seconds;
    int metric = 0;
    double metricValue;
    char cont = 'Y';
    char* path;


    if(argc == 2 && strcmp(argv[1], "-h") ==0){
        printf("This program will launch in interactive mode if run without any parameters.\n"
                "If run with parameters it will try to parse every file provided as argument.\n"
                "In that case every metric will be printed to stdouut.\n");
        exit(EXIT_SUCCESS);
    }

    if(argc > 1){
        for(int i =1; i<argc; i++){
                printf("\n\nCalculating metric for file %s\n\n", argv[i]);
                start = clock();

                parsedData = parseData(argv[i]);
                end = clock();
                seconds = (float)(end - start) / CLOCKS_PER_SEC;
                printf("Parsing data took  %.8f seconds\n", seconds);

                if(DEBUG) PrintData(parsedData);
                map = InitializeHashMap();

                start = clock();
                reducedData = GetReducedFamilyData(map, parsedData);
                end = clock();
                seconds = (float)(end - start) / CLOCKS_PER_SEC;
                printf("Generating hash map and reducing data took %.8f seconds\n", seconds);

                if(DEBUG) PrintData(reducedData);

                printf("\nMetric as iteration with Jaccard metric:\n");
                start = clock();
                metricValue = MetricOne(map);
                end = clock();
                seconds = (float)(end - start) / CLOCKS_PER_SEC;

                printf("Calculated metric = %.2f. It took  %.8f seconds\n", metricValue, seconds);

                printf("\nMetric as sum of a difference:\n");
                start = clock();
                metricValue = MetricTwo(map);
                end = clock();
                seconds = (float)(end - start) / CLOCKS_PER_SEC;
                printf("Calculated metric = %.2f. It took  %.8f seconds\n", metricValue, seconds);

                printf("\nIntuitive metric:\n");
                start = clock();
                metricValue = MetricThree(map, parsedData);
                end = clock();
                seconds = (float)(end - start) / CLOCKS_PER_SEC;
                printf("Calculated metric = %.2f. It took  %.8f seconds\n", metricValue, seconds);

                printf("\nApproximated intuitive metric:\n");
                start = clock();
                metricValue = MetricThreeApprox(map, parsedData);
                end = clock();
                seconds = (float)(end - start) / CLOCKS_PER_SEC;
                printf("Calculated metric = %.2f. It took  %.8f seconds\n", metricValue, seconds);
                FreeHashMap(map);

        }
        exit(EXIT_SUCCESS);
    }
    path =(char*)malloc(sizeof(char)*MAXPATH);
    while(cont == 'y' || cont == 'Y') {
        printf("Please input path to the text file with data: ");
        scanf("%s", path);

        printf("Please choose which metric to use (1/2/3): \n");
        printf("1. Iteration with Jaccard metric.\n");
        printf("2. Sum of a difference.\n");
        printf("4. Intuitive metric.\n");
        printf("5. Aproximated intuitive metric.\n");
        printf("6. All.\n");
        scanf("%d", &metric);

        parsedData = parseData(path);
        if(DEBUG) printf("\n\nData read\n\n");

        if(DEBUG) PrintData(parsedData);
        map = InitializeHashMap();
        if(DEBUG) printf("\n\nReducing data\n\n");
        reducedData = GetReducedFamilyData(map, parsedData);
        if(DEBUG) printf("\n\nData reduced\n\n");
        PrintData(reducedData);

        switch(metric){
            case 1:
                metricValue = MetricOne(map);
                printf("Calculated metric = %.2f\n", metricValue);
                break;
            case 2:
                metricValue = MetricTwo(map);
                printf("Calculated metric = %.0f\n", metricValue);
                break;
            case 3:
                printf("\nMetric as iteration with Jaccard metric:\n");
                metricValue = MetricOne(map);
                printf("Calculated metric = %.2f\n", metricValue);
                printf("\nMetric as sum of a difference:\n");
                metricValue = MetricTwo(map);
                printf("Calculated metric = %.0f\n", metricValue);
                break;
            case 4:
                printf("\nIntuitive metric:\n");
                metricValue = MetricThree(map, parsedData);
                printf("Calculated metric = %.2f\n", metricValue);
                break;
            case 5:
                printf("\nApproximated intuitive metric:\n");
                metricValue = MetricThreeApprox(map, parsedData);
                printf("Calculated metric = %.2f\n", metricValue);
                break;
            case 6:
                printf("\nMetric as iteration with Jaccard metric:\n");
                metricValue = MetricOne(map);
                printf("Calculated metric = %.2f\n", metricValue);

                printf("\nMetric as sum of a difference:\n");
                metricValue = MetricTwo(map);
                printf("Calculated metric = %.0f\n", metricValue);

                printf("\nIntuitive metric:\n");
                metricValue = MetricThree(map, parsedData);
                printf("Calculated metric = %.2f\n", metricValue);

                printf("\nApproximated intuitive metric:\n");
                metricValue = MetricThreeApprox(map, parsedData);
                printf("Calculated metric = %.2f\n", metricValue);

                break;
            default:
                printf("Sorry, I don't understand.\n");
        }

        FreeHashMap(map);
        printf("\n\nWould you like to calculate metrics for another file (Y/N)? ");
        scanf(" %c", &cont);
    }

    return 0;
}

int MetricTwo(HashMap* dictionary){
    int ab = 0, ba = 0;
    const char *key;
    int val;

    hashmap_foreach(key, val, dictionary) {
        if (val > 0){
            ab += val;
        }
        else if (val < 0){
            ba += (-1)*val;
        }
    }

    return ab+ba;
}

int MetricOne(HashMap* dictionary) {
    if(DEBUG) printf("Metric one\n");

    TaioSetList *listA = CreateList();
    TaioSetList *listB = CreateList();
    
    if(DEBUG) printf("Prepare lists\n");
    PrepareLists(listA, listB, dictionary);

    if(DEBUG) printf("Iterate sets\n");
    double res = IterateSets(listA, listB);
    if(DEBUG) printf("Free lists\n");

    FreeList(listA);
    FreeList(listB);

    return res;
}

void PrepareLists(TaioSetList* listA, TaioSetList* listB, HashMap* dictionary) {
    const char *key;
    int val;
    hashmap_foreach(key, val, dictionary) {
        char *tmp = (char *)malloc(sizeof(char) * (strlen(key) + 1));
        strcpy(tmp, key);

        if (val > 0) {
            for(int i = 0; i < val; i++) {
                InsertLast(listA, parseSet(tmp));
            }
        }
        else if (val < 0){
            for(int i = 0; i > val; i--) {
                InsertLast(listB, parseSet(tmp));
            }
        }

        if(tmp)
            free(tmp);
    }
}

double IterateSets(TaioSetList *A, TaioSetList *B) {
    double sum = 0;
    if(A->elemNum == 0)
        return B->elemNum;
    if(B->elemNum == 0)
        return A->elemNum;

    TaioSetListElement *X = A->Head;

    if(DEBUG) printf("Begin while\n");

    while(X) {
        if(DEBUG) printf("1\n");
        TaioSetListElement *Y = B->Head;
        if(DEBUG) printf("2\n");
        while(Y) {
            if(DEBUG) printf("3\n");
            sum += 1 - J(X->Set, Y->Set);
            if(DEBUG) printf("4\n");
            Y = Y->Next;
        }
        if(DEBUG) printf("5\n");
        X = X->Next;
    }

    return sum;
}

double J(TaioSet* X, TaioSet* Y) {
    if(DEBUG) printf("Begin J\n");
    int cut = 0;

    for(int i_x = 0; i_x < X->Count; i_x++) {
        for(int i_y = 0; i_y < Y->Count; i_y++) {
            if(DEBUG) printf("i_x = %d, i_y=%d\n");
            if(X->Numbers[i_x] == Y->Numbers[i_y]) {
                cut++;
                break;
            }
        }
    }

    double sum = X->Count + Y->Count - cut;
    return cut/sum;
}

// konwertuje rodzinę na zbiór liczb (liczba: reprezentacja binarna zbioru)
TaioSet* FamilyToSet(TaioSetFamily* family) {
    TaioSet *set = (TaioSet *)malloc(sizeof(TaioSet));

    set->Count = family->SetCount;
    set->Numbers = (int *)malloc(sizeof(int) * set->Count);

    for(int i = 0; i < set->Count; i++) {
        set->Numbers[i] = SetToBin(family->Sets[i]);
    }

    return set;
}

// konwertuje listę zbiorów na zbiór liczb (liczba: reprezentacja binarna zbioru)
TaioSet* ListToSet(TaioSetList* list) {
    TaioSet *set = (TaioSet *)malloc(sizeof(TaioSet));

    set->Count = list->elemNum;
    set->Numbers = (int *)malloc(sizeof(int) * set->Count);

    TaioSetListElement* p = list->Head;
    int i = 0;
    while(p) {
        set->Numbers[i] = SetToBin(p->Set);
        i++;
        p = p->Next;
    }

    return set;
}

// konwertuje zbiór na liczbę binarną (gdy zbiór zawiera liczbę 4, to liczba binarna ma 1 na bin[3])
int SetToBin(TaioSet* set) {
    int bin = 0;

    for(int i = 0; i < set->Count; i++) {
        bin |= 1 << (set->Numbers[i] - 1);
    }

    return bin;
}

// oblicza prostą odległość jaccarda między zbiorami
double AlternativeMetric(TaioData* data) {
    TaioSet* A = FamilyToSet(data->Family1);
    TaioSet* B = FamilyToSet(data->Family2);

    double jDist = 1 - J(A, B);

    free(A->Numbers);
    free(A);
    free(B->Numbers);
    free(B);

    return jDist;
}


// porównuje każdy zbiór w rodzinie z każdym
// zlicza ile każdy zbiór ma wspólnych elementów i ile się różniących
// metryka to 1 - ( suma_ile_różniących_się / suma_ile_takich_samych ) / (wszystkie_elementy)
// inaczej :  1 - ( trójkąt(A,B) / suma(A,B) )
double MetricThree(HashMap* dictionary, TaioData* data) {
    TaioSetList *listA_ex = CreateList();
    TaioSetList *listB_ex = CreateList();
    PrepareLists(listA_ex, listB_ex, dictionary);

    TaioSetFamily* A = data->Family1;
    TaioSetFamily* B = data->Family2;

    int common = 0, different = 0; 

    if(A->SetCount == 0 && B->SetCount == 0) return 0;
    if(A->SetCount == 0 || B->SetCount == 0) return 1;

    for(int i_a = 0; i_a < A->SetCount; i_a++) {
        TaioSet* X = A->Sets[i_a];

        for(int i_b = 0; i_b < B->SetCount; i_b++) {
            TaioSet* Y = B->Sets[i_b];

            if(strcmp(X->Name, Y->Name) == 0) {
                for(int i_x = 0; i_x < X->Count; i_x++) {
                    for(int i_y = 0; i_y < Y->Count; i_y++) {
                        int x = X->Numbers[i_x];
                        int y = Y->Numbers[i_y];

                        if(x == y) {
                            common++;
                            break;
                        } 
                    }
                }
            }
        }
    }

    TaioSetListElement* A_ex = listA_ex->Head;
    while(A_ex) {
        TaioSet* X = A_ex->Set;
        TaioSetListElement* B_ex =  listB_ex->Head;
        while(B_ex) {
            TaioSet* Y = B_ex->Set;
            int max_iy = 0;
            bool found_x = false;
            bool found_y = false;

            for(int i_x = 0; i_x < X->Count; i_x++) {
                found_x = false;
                
                int prev_x = X->Numbers[(i_x == 0 ? 0 : i_x - 1)];

                for(int i_y = 0; Y->Numbers[i_y] <= X->Numbers[i_x] && i_y < Y->Count; i_y++) {
                    found_y = false;
                    int x = X->Numbers[i_x];
                    int y = Y->Numbers[i_y];
                    
                    max_iy = max_iy < i_y ? i_y : max_iy;

                    if(x == y) {
                        found_x = true;
                        common++;
                        break;
                    } 
                    else {
                        if(y > prev_x && i_x > 0) {
                            for(int i_xy = i_x; y <= X->Numbers[i_xy] && i_xy > 0; i_xy--) {
                                int xy = X->Numbers[i_xy];

                                if(xy == y) {
                                    found_y = true;
                                    break;
                                }
                            }

                            if(!found_y) {
                                different++;
                            }
                        }
                        else if (i_x == 0) {
                            if(y != X->Numbers[i_x])
                                different++;
                        }
                    }
                }

                if(!found_x) {
                    different++;
                }
                if(i_x == X->Count - 1) {
                    different += (Y->Count - 1) - max_iy; // greatest element in X is less than greates elements in Y (thus those elements in Y are different)
                }
            }
            B_ex = B_ex->Next;
        }
        A_ex = A_ex->Next;
    }

    FreeList(listA_ex);
    FreeList(listB_ex);

    if(common == 0) return 1;
    return different*1.0 / (common*1.0 + different*1.0);
}

// konwetuje rodzinę na zbiór liczb binarnych
// porównując każdy zbiór z każdym (aka każdy zbiór w rodzinie z każdym)
// zlicza ile dane liczby binarne mają takich samych 'jedynek' a ile różnych (aka zlicza ile każdy zbiór ma wspólnych elementów i ile różniących się)
// metryka to 1 - ( suma_ile_różniących_się / suma_jedynek )
double MetricThreeApprox(HashMap* dictionary, TaioData* data) {
    TaioSetList *listA = CreateList();
    TaioSetList *listB = CreateList();
    PrepareLists(listA, listB, dictionary);

    TaioSet* A_ex = ListToSet(listA);
    TaioSet* B_ex = ListToSet(listB);
    TaioSet* A = FamilyToSet(data->Family1);
    TaioSet* B = FamilyToSet(data->Family2);

    int common = 0, different = 0; 

    if(A->Count == 0 && B->Count == 0) return 0;
    if(A->Count == 0 || B->Count == 0) return 1;

    for(int i_a = 0; i_a < A->Count; i_a++) {
        int a = A->Numbers[i_a];
        for(int i_b = 0; i_b < B->Count; i_b++) {
            int b = B->Numbers[i_b];
            if(a == b) {
                int com_1 = a&b;
                common += CountOnes(com_1);
            }
        }
    }

    for(int i_a_ex = 0; i_a_ex < A_ex->Count; i_a_ex++) {
        int a_ex = A_ex->Numbers[i_a_ex];
        for(int i_b_ex = 0; i_b_ex < B_ex->Count; i_b_ex++) {
            int b_ex = B_ex->Numbers[i_b_ex];
            int diff_1 = a_ex^b_ex;
            int com_1 = a_ex&b_ex;
            different += CountOnes(diff_1);
            common += CountOnes(com_1);
        }
    }

    free(A->Numbers);
    free(A);
    free(B->Numbers);
    free(B);
    FreeList(listA);
    FreeList(listB);

    if(common == 0) return 1;
    return different*1.0 / (common + different);
}

int CountOnes(int N) {
    int count = 0;
 
    while (N > 0) {
        if (N & 1) {
            count++;
        }

        N = N >> 1;
    }
 
    return count;
}
