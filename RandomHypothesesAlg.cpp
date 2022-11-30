#include "RandomHypothesesAlg.h"

#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <cstdlib>
#include <cmath>

using std::ceil;
using std::vector;
using std::cout;
using std::min_element;
using std::endl;
using std::set;

// first index is for objects, i.e. contains vectors of attributes
vector< vector<int> > g_ContextMatrix;
vector<int> g_EntireAttributeSet;
vector<int> g_EntireObjectSet;
vector<int> g_TargetAttribute;

vector< vector<int> > g_HypothesisExtents;
vector< vector<int> > g_HypothesisIntents;
vector<int> g_HypothesisMarkOfClass;

int g_NumberOfAttributes;
int g_NumberOfObjects;

int g_NumberOfRepeatedHypotheses;
bool g_Verbosity;

void PrintVector(const vector<int> & vec) {
  for (int index = 0; index < vec.size(); ++index) {
    cout << vec[index] << ' ';
  }
}

void PrintHypothesis(const vector<int> & extent,
                     const vector<int> & intent) {
  g_HypothesisExtents.push_back(extent);
  g_HypothesisIntents.push_back(intent);
  int target_attribute_of_first_object = g_TargetAttribute[extent[0]];
  g_HypothesisMarkOfClass.push_back(target_attribute_of_first_object);

  if (g_Verbosity) {
    cout << (g_TargetAttribute[extent[0]] ? "+" : "-") << " hypothesis" << endl;

    cout << "Extent: ";
    PrintVector(extent);
    cout << endl;

    cout << "Intent: ";
    PrintVector(intent);
    cout << endl;
  }
}

void SetIntersection(const vector<int> & vector1, 
                     const vector<int> & vector2,
                     vector<int> * result) {
  int index1 = 0, index2 = 0;
  int vector1_size = vector1.size();
  int vector2_size = vector2.size();

  while (index1 < vector1_size && index2 < vector2_size)
  {
    while (index1 < vector1_size) {
      if (vector1[index1] < vector2[index2]) {
        ++index1;
      } else {
        break;
      }
    }

    if(index1 >= vector1_size) {
      break;
    }

    while (index2 < vector2_size) {
      if (vector1[index1] > vector2[index2]) {
        ++index2;
      } else {
        break;
      }
    }

    // index1 is already checked
    if (index2 < vector2_size) {
      if (vector1[index1] == vector2[index2]) {
        result->push_back(vector1[index1]);
        ++index1;
        ++index2;
      }
    }
  }
}

// element of vector2 are subtracted from vector1
void SetDifference(const vector<int> & vector1, 
                   const vector<int> & vector2,
                   vector<int> * result) {
  int index1 = 0, index2 = 0;
  int vector1_size = vector1.size();
  int vector2_size = vector2.size();

  if (vector2_size == 0) {
    result->assign(vector1.begin(), vector1.end());
    return;
  }

  if (vector1_size == 0) {
    return;
  }

  while (index1 < vector1_size)
  {
    while (index1 < vector1_size) {
      if (vector1[index1] < vector2[index2]) {
        result->push_back(vector1[index1]);
        ++index1;
      } else {
        break;
      }
    }

    if(index1 >= vector1_size) {
      break;
    }

    while (index2 < vector2_size) {
      if (vector1[index1] > vector2[index2]) {
        ++index2;
      } else {
        break;
      }
    }

    // index1 is already checked
    if (index2 < vector2_size) {
      if (vector1[index1] == vector2[index2]) {
        ++index1;
        ++index2;
      }
    }
    if (index2 >= vector2_size) {
      while (index1 < vector1_size) {
        result->push_back(vector1[index1]);
        ++index1;
      }
      break;
    }
  }
}

void ComputeObjectStroke(int object, 
                         vector<int> * object_stroke) {
  for (int attribute = 0; attribute < g_NumberOfAttributes; ++attribute) {
    if (g_ContextMatrix[object][attribute] == 1) {
      object_stroke->push_back(attribute);
    }
  }
}

void ComputeObjectSetClosure(const vector<int> & object_set,
                                const vector<int> & object_set_stroke,
                                vector<int> * object_set_closure) {
  int index_in_object_set = 0;

  for (int object = 0; object < g_NumberOfObjects; ++object) {
    
    if (index_in_object_set < object_set.size()) {
      if (object == object_set[index_in_object_set]) {
        object_set_closure->push_back(object);
        ++index_in_object_set;
        continue;
      }
    }

    bool all_attributes_in_set_have_object = true;
    for(int index = 0; index < object_set_stroke.size(); ++index) {
      int attribute = object_set_stroke[index];
      if (g_ContextMatrix[object][attribute] != 1) {
        all_attributes_in_set_have_object = false;
        break;
      }
    }

    if (all_attributes_in_set_have_object) {
      object_set_closure->push_back(object);
    }
  }
}

bool IsHypothesis(const vector<int> & object_set) {
  // all objects must belong to the same class
  if (object_set.size() == 0) {
    return false;
  }

  int target_attribute_of_first_object = g_TargetAttribute[object_set[0]];
  for (int index = 0; index < object_set.size(); ++index) {
    int object = object_set[index];
    if (g_TargetAttribute[object] != target_attribute_of_first_object) {
      return false;
    }
  }
  return true;
}

bool AddObjectRecursively(const vector<int> & current_object_set,
                             int object_to_add,
                             const vector<int> & current_object_set_stroke) {

  //cout << "+" << endl;

  vector<int> extended_object_set(current_object_set);
  extended_object_set.push_back(object_to_add);

  vector<int> object_to_add_stroke;
  ComputeObjectStroke(object_to_add, &object_to_add_stroke);

  //cout << "*" << endl;

  vector<int> extended_object_set_stroke;
  SetIntersection(current_object_set_stroke, 
                  object_to_add_stroke, 
                  &extended_object_set_stroke);

  //cout << "?" << endl;

  vector<int> extended_object_set_closure;
  ComputeObjectSetClosure(extended_object_set,
                          extended_object_set_stroke,
                          &extended_object_set_closure);

  bool current_closure_is_hypothesis = IsHypothesis(extended_object_set_closure);

  //cout << "%" << endl;

  if (!current_closure_is_hypothesis) {
    return false;
  }

  vector<int> objects_not_in_current_closure;
  SetDifference(g_EntireObjectSet, extended_object_set_closure, &objects_not_in_current_closure);
  // you should only consider objects whcih are the same class with those existing in the current extent
  // cause otherwise you will generate non-hypothesis closure right away

  if (objects_not_in_current_closure.size() != 0)
  {
    int next_random_object_index = rand() % objects_not_in_current_closure.size();
    int next_random_object = objects_not_in_current_closure[next_random_object_index];

    bool there_is_hypothesis_further = 
      AddObjectRecursively(extended_object_set_closure, 
                           next_random_object, 
                           extended_object_set_stroke);

    if (there_is_hypothesis_further) {
      return true;
    }

    // random object failed, trying to add object
    for (int next_object_index = 0; 
         next_object_index < objects_not_in_current_closure.size();
         ++next_object_index) {
      if (next_object_index == next_random_object_index) {
        continue;
      }
      next_random_object = objects_not_in_current_closure[next_object_index];
      bool there_is_hypothesis_further = 
        AddObjectRecursively(extended_object_set_closure, 
                             next_random_object, 
                             extended_object_set_stroke);
      if (there_is_hypothesis_further) {
        return true;
      }
    }
  }
  PrintHypothesis(extended_object_set_closure, extended_object_set_stroke);
  
  return true;
}

bool LastHypothesisIsNotInList() {
  vector<int> & last_hypothesis = g_HypothesisIntents.back();
  for (int index = 0; index + 1 < g_HypothesisIntents.size(); ++index) {
    if (g_HypothesisIntents[index] == last_hypothesis) {
      return false;
    }
  }
  return true;
}

void GetRandomMinimalHypotheses(int min_number_of_objects_to_cover) {
  vector<int> empty_object_set;

  set<int> all_covered_objects;
  int number_of_repeated_hypothesis = 0;

  while (all_covered_objects.size() < min_number_of_objects_to_cover) {
    int first_object_to_add = rand() % g_NumberOfObjects;
    bool there_is_hypothesis = 
      AddObjectRecursively(empty_object_set, first_object_to_add, g_EntireAttributeSet);

    if (there_is_hypothesis) {
      bool current_hypothesis_is_unique = LastHypothesisIsNotInList();
      if (current_hypothesis_is_unique) {
        vector<int> objects_covered_by_current_hypothesis = g_HypothesisExtents.back();
        // TODO: is it the best?

        all_covered_objects.insert(objects_covered_by_current_hypothesis.begin(),
                                   objects_covered_by_current_hypothesis.end());
        if (g_Verbosity) {
          cout << endl;
        }
      } else {
        g_HypothesisExtents.pop_back();
        g_HypothesisIntents.pop_back();
        g_HypothesisMarkOfClass.pop_back();

        ++number_of_repeated_hypothesis;
        if (g_Verbosity) {
          cout << " Found hypothesis already in the list" << endl << endl;
        }
      }
    } else {
      if (g_Verbosity) {
        // Means that one of the randomly inserted objectes could not be covered by hypothesis
        cout << " Attempt to find hypothesis failed." <<endl;
      }
    }
  }
  g_NumberOfRepeatedHypotheses = number_of_repeated_hypothesis;
  
  //cout << " Objects covered: " << all_covered_objects.size() << endl;
}

vector< vector<int> > GetHypothesesRandomHypothesesAlg() {
  return g_HypothesisIntents;
}

vector<int> GetHypothesesMarkOfClassRandomHypothesesAlg() {
  return g_HypothesisMarkOfClass;
}

void InitializeRandomHypothesesAlg(vector< vector<int> > & context_matrix, vector<int> & target_attribute) {
  
  g_NumberOfAttributes = context_matrix.size();
  g_NumberOfObjects = context_matrix[0].size();
  g_NumberOfRepeatedHypotheses = 0;
  g_Verbosity  = false;

  
  g_EntireAttributeSet.clear();
  g_EntireObjectSet.clear();
  
  for (int attribute = 0; attribute < g_NumberOfAttributes; ++attribute) {
    g_EntireAttributeSet.push_back(attribute);
  }
 
  for (int object = 0; object < g_NumberOfObjects; ++object) {
    g_EntireObjectSet.push_back(object);
  }

  g_TargetAttribute.clear();
  g_TargetAttribute = target_attribute;

  g_ContextMatrix.clear();

  for (int object = 0; object < g_NumberOfObjects; ++object) {
    vector<int> current_object;
    for (int attribute = 0; attribute < g_NumberOfAttributes; ++attribute) {
      current_object.push_back(context_matrix[attribute][object]);
    }
    g_ContextMatrix.push_back(current_object);
  }

  g_HypothesisExtents.clear();
  g_HypothesisIntents.clear();
  g_HypothesisMarkOfClass.clear();
}

// it is a slighlty changed copy of 
// void GetRandomMinimalHypotheses(int min_number_of_objects_to_cover) 
int GetNumberOfObjectsPossibleToCover() {

    int number_of_objects_possible_to_cover = 0;
  for (int object = 0; object < g_NumberOfObjects; ++object) {
    vector<int> object_stroke;
    ComputeObjectStroke(object, &object_stroke);
    vector<int> object_closure;
    ComputeObjectSetClosure(vector<int>(1, object), object_stroke, &object_closure);
    if (IsHypothesis(object_closure)) {
      ++number_of_objects_possible_to_cover;
    }
  }
  return number_of_objects_possible_to_cover;
}


void RandomHypothesesAlg(int min_number_of_objects_to_cover, int & number_of_repeated_hypothesis) {
  //cout << "RandomHypothesesAlg." << endl;
  GetRandomMinimalHypotheses(min_number_of_objects_to_cover);
  number_of_repeated_hypothesis = g_NumberOfRepeatedHypotheses;
  //cout << " Number of hypotheses: " << g_HypothesisIntents.size() << endl;
}
