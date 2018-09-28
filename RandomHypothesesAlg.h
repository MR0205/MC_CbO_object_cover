#ifndef __RANDOM_HYPOTHESES_ALG__
#define __RANDOM_HYPOTHESES_ALG__

#include <vector>
using std::vector;

void InitializeRandomHypothesesAlg(vector< vector<int> > & context_matrix, vector<int> & target_attribute);
void RandomHypothesesAlg(int min_number_of_objects_to_cover, int & number_of_repeated_hypothesis);
vector< vector<int> > GetHypothesesRandomHypothesesAlg();
vector<int> GetHypothesesMarkOfClassRandomHypothesesAlg();
void SetDifference(const vector<int> & vector1, const vector<int> & vector2, vector<int> * result);
int GetNumberOfObjectsPossibleToCover();

#endif