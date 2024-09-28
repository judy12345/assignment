#include <iostream>
#include <string>
#include <tuple>
#include <set>
#include <array>
#include <map>

using namespace std;

enum Level {
  EQUAL,
  LESSTHAN,
  GREATERTHAN
};

template <size_t arity>
class relation
{
private:
    int tupleCount;
    set<array<int, arity>> dataBuffer;

public:
    relation() {tupleCount = 0;}
    relation(const char *filename);
    
    void loadFromFile(const char *filename);
    void saveToFile(const char *filename);
    void printRelation();
    
    // These functions will be needed in the core RA operatior functions (projection, selection, cross-product and equi-join)
    set<array<int, arity>> getDataBuffer() {return dataBuffer;}
    void setDataBuffer(set<array<int, arity>> dataBuffer) {this->dataBuffer = dataBuffer;}
    int getTupleCount() {return tupleCount;}
    void setTupleCount(int tupleCount) {this->tupleCount = tupleCount;}
};

/*
 * The data on file is comprised of "n" rows and "arity" columns 
 * The constructor loads data from a raw (binary) file.
 * The binary file is composed of n x arity integers.
 * So, if the size of the file is n bytes, then there are n/4 entries in the file.
 *
 * This function loads the data from the disk and populate the dataBuffer (which is a set of arrays). 
 */
template <size_t arity>
void relation<arity>::loadFromFile(const char* filename)
{
    FILE* pFile;
    long fileSize;
    int* buffer;
    size_t result;

    // ʹ�� fopen_s ���� fopen
    errno_t err = fopen_s(&pFile, filename, "rb");
    if (err != 0 || pFile == NULL)
    {
        fputs("File error", stderr);
        exit(1);
    }

    // ��ȡ�ļ���С�����ֽ�Ϊ��λ��
    fseek(pFile, 0, SEEK_END);
    fileSize = ftell(pFile);
    rewind(pFile);

    // �����ڴ��԰��������ļ�
    buffer = (int*)malloc(fileSize);
    if (buffer == NULL)
    {
        cout << "Memory error" << endl;
        fclose(pFile);  // �ǵùر��ļ�
        exit(1);
    }

    // ���ļ����ݸ��Ƶ���������
    result = fread(buffer, 1, fileSize, pFile);
    if (result != fileSize)
    {
        cout << "File reading error" << endl;
        free(buffer);   // �ͷ��ڴ�
        fclose(pFile);  // �ر��ļ�
        exit(1);
    }

    // �ر��ļ�
    fclose(pFile);

    // ��� dataBuffer ���ݽṹ�������ϵ����
    for (int i = 0; i < fileSize / sizeof(int); i += arity)
    {
        std::array<int, arity> arr;
        for (size_t j = 0; j < arity; ++j)
            arr[j] = buffer[i + j];
        dataBuffer.insert(arr);
    }

    tupleCount = dataBuffer.size();

    // �ͷŻ�����
    free(buffer);
}



template <size_t arity>
void relation<arity>::saveToFile(const char* filename)
{
    FILE* pFile;

    // ʹ�� fopen_s ���ļ���ע���һ�������� FILE* ��ָ��
    errno_t err = fopen_s(&pFile, filename, "wb");
    if (err != 0 || pFile == NULL)
    {
        fputs("File error", stderr);
        exit(1);
    }

    // ���� dataBuffer ���ݽṹ��д�뵽����
    for (const auto& tuple : dataBuffer)  // ʹ�������Ա��ⲻ��Ҫ�Ŀ���
    {
        int temp[arity];
        for (int i = 0; i < arity; i++)
            temp[i] = tuple[i];
        fwrite(temp, sizeof(int), arity, pFile);
    }

    // �ر��ļ�
    fclose(pFile);
}

template <size_t arity>
relation<arity>::relation(const char *filename)
{
    loadFromFile(filename);
}

template <size_t arity>
void relation<arity>::printRelation()
{
    cout << "Number of tuples in the relation: " << dataBuffer.size() << endl;
    
    // Note there are many ways of iterating through the set data structure
    // Method 1
    auto it = dataBuffer.begin();
    for (int i = 0; i < dataBuffer.size(); i++)
    {
        std::array<int, arity> arr = *it;
        for (int j=0; j < arity; j++)
        {
            if (j < arity-1)
                cout << arr[j] << " ";
            else
                cout << arr[j] << endl;
        }
        it++;
    }

    // Method 2 (I showed this method in the class)
    for (const auto& tuple : dataBuffer)
    {
        for (int j=0; j < arity; j++)
        {
            //if (j < arity-1)
            //    cout << tuple[j] << " ";
            //else
            //    cout << tuple[j] << endl;
        }
    }
}



/*
 * Template parameters:
 *  arity - the arity of both input and output relation
 *   
 * Function parameters: 
 *  inputRelation: this is the input relation on which the selection operation will be applied
 *  attributeIndex: it is the index of the attribute column on which the selection operation is to be applied
 *  operation: it can be EQUAL or LESSTHAN or GREATERTHAN
 *  operand: it the the value against which the attribute value is compared with using the "operation".
 * 
 *  For example:
 *      selection<2>(rel2Arity, 1, GREATERTHAN, 500) -- rel2Arity is a 2-arity (i.e. 2 columns) relation, 
 *                                                  on which we are applying the selection condition 
 *                                                  on the second column, and we keep a row/tuple if the
 *                                                  second column value is greater than 500
 *      selection<4>(rel4Arity, 3, EQUAL, 200) -- rel4Arity is a 4-arity (i.e. 4 columns) relation, 
 *                                                  on which we are applying the selection condition 
 *                                                  on the fourth column, and we keep a row/tuple if the
 *                                                  fourth column value is equal to 200
 * 
 *  Note:
 *          The arity of the input and outpur relations are the same, therefore the template has only one arity parameter
 *          attributeIndex staarts from index 0, therefore its value cannot be greater than or equal to inputArity
 */
template<size_t arity>
static relation<arity> selection(relation<arity> inputRelation, int attributeIndex, int operation, int operand) {
    if (attributeIndex >= arity) {
        cout << "You are trying to do a selection on an invalid attribute" << endl;
        exit(1);
    }

    auto outputRelation = relation<arity>();
    set<array<int, arity>> outputDataBuffer;
    // Iterate through input relation's tuples
    for (const auto& tuple : inputRelation.getDataBuffer()) {
        bool condition = false;

        switch (operation) {
        case EQUAL:
            condition = (tuple[attributeIndex] == operand);
            break;
        case LESSTHAN:
            condition = (tuple[attributeIndex] < operand);
            break;
        case GREATERTHAN:
            condition = (tuple[attributeIndex] > operand);
            break;
        }

        // Add tuple to the output relation if it satisfies the condition
        if (condition) {
            outputDataBuffer.insert(tuple);
        }
    }

    // Set the tuple count for the output relation
    outputRelation.setDataBuffer(outputDataBuffer);
    outputRelation.setTupleCount(outputDataBuffer.size());

    return outputRelation;
}



/*
 * Template parameters:
 *  inputArity - the arity of the input relation
 *  outputArity - the arity of the output relation
 *   
 * Function parameters: 
 *  inputRelation: this is the input relation on which the projection operation will be applied 
 *  indicesOfAttributesToKeepArray: this array will store the indices of the attributes that will be kept
 *                                  the length of the above array is the same as outputArity
 *  
 *  For example:
 *      int indicesOfAttributesToKeepArray[1] = {1};
 *      auto rel1Arity = projection<2, 1>(rel2Arity, indicesOfAttributesToKeepArray) -  
 *                                                  rel2Arity is a 2-arity (i.e. 2 columns) relation, 
 *                                                  on which we are applying the projection operation.
 *                                                  Out of the two columns, we will keep the second column
 *                                                  as specified by te indicesOfAttributesToKeepArray array {1}.
 *                                                  
 * 
 *      int indicesOfAttributesToKeepArray[1] = {0, 2, 4};
 *      auto rel3Arity = projection<5, 3>(rel5Arity, indicesOfAttributesToKeepArray) -
 *                                                  rel5Arity is a 5-arity (i.e. 5 columns) relation, 
 *                                                  on which we are applying the projection operation. 
 *                                                  Out of the five columns, we will keep the first, third and the fifth column
 *                                                  as specified by te indicesOfAttributesToKeepArray array {0, 2, 4}.
 * 
 *  Note:
 *          The arity of the input and outpur relations are not going to be same, therefore the template has only two arity parameter
 *          The number of columns we are going to keep is the same as the arity of the output relation, and therefore the size of the
 *          indicesOfAttributesToKeepArray is outputArity
 */
template<size_t inputArity, size_t outputArity>
static relation<outputArity> projection(relation<inputArity> inputRelation, int* indicesOfAttributesToKeepArray) {
    if (inputArity < outputArity) {
        cout << "You are trying to keep more columns than what exists in the relation" << endl;
        exit(1);
    }

    auto outputRelation = relation<outputArity>();
    set<array<int, outputArity>> outputDataBuffer;
    // Iterate through input relation's tuples
    for (const auto& tuple : inputRelation.getDataBuffer()) {
        array<int, outputArity> projectedTuple;

        // Populate projected tuple with specified columns
        for (size_t i = 0; i < outputArity; ++i) {
            projectedTuple[i] = tuple[indicesOfAttributesToKeepArray[i]];
        }
        outputDataBuffer.insert(projectedTuple);
       
        // Insert the projected tuple into the output relation
       
    }

    // Set the tuple count for the output relation
    outputRelation.setDataBuffer(outputDataBuffer);
    outputRelation.setTupleCount(outputDataBuffer.size());

    return outputRelation;
}


/*
 * 
 * Cross-product (cartesian-product) takes as input two relation, inputRelation1 and inputRelation2
 * Template parameters:
 *  inputArity1 - the arity of the input relation inputRelation1
 *  inputArity2 - the arity of the input relation inputRelation2
 *   
 * Function parameters: 
 *  inputRelation1: the first input relations
 *  inputRelation2: the second input relations
 * 
 *  For example:
 *      auto rel5Arity = crossProduct<2, 3>(rel2Arity, rel3Arity) -  
 *                                                  rel2Arity is a 2-arity (i.e. 2 columns) relation, 
 *                                                  and, rel3Arity is a3-arity (i.e. 3 columns) relation
 *                                                  The output is going to be a 5 arity relation
 *                                                  
 * 
 *  Note:
 *          The arity of the two input relations do not have to be same, therefore the template has two arity parameters for the two input relations
 *          The arity of the output relation can be computed using the arities of the input relations (inputArity1 + inputArity2)
 */
template <size_t inputArity1, size_t inputArity2>
static relation<inputArity1 + inputArity2> crossProduct(relation<inputArity1> inputRelation1, relation<inputArity2> inputRelation2) {
    auto outputRelation = relation<inputArity1 + inputArity2>();
    set<array<int, inputArity1 + inputArity2>> outputDataBuffer;

    for (const auto& tuple1 : inputRelation1.getDataBuffer()) {
        for (const auto& tuple2 : inputRelation2.getDataBuffer()) {
            array<int, inputArity1 + inputArity2> combinedTuple;

            for (size_t i = 0; i < inputArity1; ++i) {
                combinedTuple[i] = tuple1[i];
            }
            for (size_t i = 0; i < inputArity2; ++i) {
                combinedTuple[inputArity1 + i] = tuple2[i];
            }
            outputDataBuffer.insert(combinedTuple);
        }
    }
    outputRelation.setDataBuffer(outputDataBuffer);
    outputRelation.setTupleCount(outputDataBuffer.size());

    return outputRelation;
}



/*
 * 
 * Equi-Join
 * Template parameters:
 *  inputArity1 - the arity of the input relation inputRelation1
 *  inputArity2 - the arity of the input relation inputRelation2
 *   
 * Function parameters: 
 *  inputRelation1: the first input relations
 *  inputRelation2: the second input relations
 *  joinColumnIndexLength - the number of columns on which the join (equality) will be performed
 *  relation1JoinColumnIndexArray - an array that stores the indices of the columns of relation 1 on which join will be performed
 *  relation2JoinColumnIndexArray - an array that stores the indices of the columns of relation 2 on which join will be performed 
 *  
 * 
 *  For example:
 *      int relation1JoinIndex[1] = {0};
 *      int relation2JoinIndex[1] = {1};
 *      auto rel5Arity = equiJoinQuadratic<2, 3>(rel2Arity, rel3Arity, 1, relation1JoinIndex, relation2JoinIndex) -  
 *                                                  rel2Arity is a 2-arity (i.e. 2 columns) relation, 
 *                                                  and, rel3Arity is a3-arity (i.e. 3 columns) relation
 *                                                  The output is going to be a 5 arity relation (rel5Arity).
 *                                                  Equality will be checked on one column. The first column of relations1,
 *                                                  the second column of relation2.
 *                                                  
 * 
 *  Note:
 *          The arity of the two input relations do not have to be same, therefore the template has two arity parameters for the two input relations
 *          The arity of the output relation can be computed using the arities of the input relations (inputArity1 + inputArity2)
 */
template <size_t inputArity1, size_t inputArity2>
static relation<inputArity1 + inputArity2> equiJoinQuadratic(relation<inputArity1> inputRelation1, relation<inputArity2> inputRelation2,
    int joinColumnIndexLength, int* relation1JoinColumnIndexArray, int* relation2JoinColumnIndexArray) {
    auto outputRelation = relation<inputArity1 + inputArity2>();
    set<array<int, inputArity1 + inputArity2>> outputDataBuffer;

    for (const auto& tuple1 : inputRelation1.getDataBuffer()) {
        for (const auto& tuple2 : inputRelation2.getDataBuffer()) {
            bool match = true;

            for (int i = 0; i < joinColumnIndexLength; ++i) {
                if (tuple1[relation1JoinColumnIndexArray[i]] != tuple2[relation2JoinColumnIndexArray[i]]) {
                    match = false;
                    break;
                }
            }

            if (match) {
                array<int, inputArity1 + inputArity2> combinedTuple;

                for (size_t i = 0; i < inputArity1; ++i) {
                    combinedTuple[i] = tuple1[i];
                }
                for (size_t i = 0; i < inputArity2; ++i) {
                    combinedTuple[inputArity1 + i] = tuple2[i];
                }
                outputDataBuffer.insert(combinedTuple);
            }
        }
    }
    outputRelation.setDataBuffer(outputDataBuffer);
    outputRelation.setTupleCount(outputDataBuffer.size());

    return outputRelation;
}






int main(int argc, char** argv) {
    

    // Please write your own test cases!!!
    // [HERE]


    /// The following code will be used to verify the correctness of your code
    /// These queries will generate file of certain size, and content that we will use to verify your code.

    /* 
     * case 1
     * The input data is designed in a way such that the output of the join will be equivalent to a cross product 
     * the fifth column (index {4}) of the first relation is all 5s
     * the first column (index {0}) of the second relation is all 5s
     */
    int case1JoinColumnIndexRelation1[1] = {4};
    int case1JoinColumnIndexRelation2[1] = {0};
    auto case1InputRelation1 = relation<5>("./data/case1Input1");
    auto case1InputRelation2 = relation<3>("./data/case1Input2"); 
    auto worstCaseEquiJoinRelation = equiJoinQuadratic<5, 3>(
        case1InputRelation1,                /* First relation */
        case1InputRelation2,                /* Second relation */
        1,                                  /* Number of columns on both relations, we are doing a comparision operator on */
        case1JoinColumnIndexRelation1       /* Array storing the indices on which the join operation of the first relation is done.*/, 
        case1JoinColumnIndexRelation2       /* Array storing the indices on which the join operation of the second relation is done.*/);
    
    case1InputRelation1.printRelation();
    case1InputRelation2.printRelation();
    worstCaseEquiJoinRelation.printRelation();
    worstCaseEquiJoinRelation.saveToFile("case1Output");
    // Verify for correctness
    if (worstCaseEquiJoinRelation.getTupleCount() != case1InputRelation1.getTupleCount() * case1InputRelation2.getTupleCount())
        cout << "Error in your implementation!!!!" << endl 
             << "The output relation should be of size " 
             << (case1InputRelation1.getTupleCount() * case1InputRelation2.getTupleCount())
             << ", Yours is " << worstCaseEquiJoinRelation.getTupleCount() << endl;
    else
        cout << "Case 1, test passed" << endl;

    cout << "--------------------------------------------------------" << endl;
    /* 
     * case 2
     * The input data is designed in a way such that the output of the join will have zero tuples
     * The zero'th column does not have any matching values between the two relations. 
     */
    int case2JoinColumnIndexRelation1[1] = {0};
    int case2JoinColumnIndexRelation2[1] = {0};
    auto case2InputRelation1 = relation<4>("./data/case2Input1");
    auto case2InputRelation2 = relation<4>("./data/case2Input2");
    auto zeroOutputEquiJoinRelation = equiJoinQuadratic<4, 4>(
        case2InputRelation1, 
        case2InputRelation2, 
        1, 
        case2JoinColumnIndexRelation1, 
        case2JoinColumnIndexRelation2);
    case2InputRelation1.printRelation();
    case2InputRelation2.printRelation();
    zeroOutputEquiJoinRelation.printRelation();
    zeroOutputEquiJoinRelation.saveToFile("case2Output");
    if (zeroOutputEquiJoinRelation.getTupleCount() != 0)
        cout << "Error in your implementation!!!!" << endl
             << "The output relation should have 0 tuples." << endl;
    else
        cout << "Case 2, test passed" << endl;

    cout << "--------------------------------------------------------" << endl;
    /*
     * Case 3
     * Cross-product
     */
    auto case3InputRelation1 = relation<3>("./data/case3Input1");
    auto case3InputRelation2 = relation<3>("./data/case3Input2");
    auto crossProductRelation = crossProduct<3, 3>(
        case3InputRelation1, 
        case3InputRelation2);
    case3InputRelation1.printRelation();
    case3InputRelation2.printRelation();
    crossProductRelation.printRelation();
    crossProductRelation.saveToFile("case3Output");
    if (crossProductRelation.getTupleCount() != case3InputRelation1.getTupleCount() * case3InputRelation2.getTupleCount())
        cout << "Error in your implementation!!!!" << endl;
    else
        cout << "Case 3, test passed" << endl;

    cout << "--------------------------------------------------------" << endl;
    /*
     * Case 4
     * Selection - this case is designed to select half the number of rows
     */
    auto case4InputRelation = relation<3>("./data/case4Input");
    auto selectionRelation = selection<3>(case4InputRelation, 0, EQUAL, 100);
    case4InputRelation.printRelation();
    selectionRelation.printRelation();
    selectionRelation.saveToFile("case4Output");
    if (selectionRelation.getTupleCount() != case4InputRelation.getTupleCount()/2)
        cout << "Error in your implementation!!!!" << endl;
    else
        cout << "Case 4, test passed" << endl;

    cout << "--------------------------------------------------------" << endl;
    /*
     * Case 5
     * The output relation drops one of the columns
     */
    auto case5InputRelation = relation<3>("./data/case5Input");
    int case5ProjectionColumnIndex[2] = {0, 2};
    auto projectionRelationA = projection<3, 2>(case5InputRelation, case5ProjectionColumnIndex);
    case5InputRelation.printRelation();
    projectionRelationA.printRelation();
    projectionRelationA.saveToFile("case5Output");
}
