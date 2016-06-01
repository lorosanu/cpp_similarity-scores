#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <map>
#include <algorithm>

using namespace std;

/**
 * @brief  replace the punctuation characters with whitespaces in given text
 * @param  string : the reference text
 * @return string : the text without punctuation
*/
string removePunctuation(string input);


/**
 * @brief  split text into words (tokenize by whitespaces)
 * @param  string : the reference text
 * @return map<string,int> : a map of words and their number of occurrences
*/
map<string,int> getUniqueWords(string text);


/**
 * @brief  compute the term frequency (TF) of each reference word in each document [ tf(t,d)= freq(t,d)/|t':t' in d|]
 * @param  map<string,int> : the reference words
 * @param  string[] : the documents;
 * @param  int : the number of documents
 * @return vector<map<string, double> > : the array of maps TF(rW,Di) of each rW reference word in each Di document
*/
vector<map<string, double> > computeTF(map<string,int> refWords, string Docs[], int nDocs);


/**
 * @brief  compute the inverse document frequency (IDF) of reference words [ idf(t,D)=log(|D|/|{d:t in D}|) ]
 * @param  map<string,int> : the list of unique reference words
 * @param  string[] : the documents
 * @param  int : the number of documents
 * @return map<string, double> : the map IDF(rW) giving the IDF score of each rW reference word
*/
map<string, double> computeIDF(map<string,int> refWords, string Docs[], int nDocs);


/**
 * @brief  compute the TF-IDF score of each reference word in each document [ tfidf(t, d, D) = tf(t,d) × idf(t,D) ]
 * @param  map<string,int> : the list of unique reference words
 * @param  vector<map<string, double> > : the TF scores
 * @param  map<string, double> : the IDF scores
 * @return vector<map<string, double> > : the array of maps TFIDF(rW,Di) of each rW reference word in each Di document
*/
vector<map<string, double> > computeTFIDF(map<string,int> refWords, vector<map<string, double> > tf, map<string, double> idf);


/**
 * @brief  compute the sum of TF-IDF scores for each document
 * @param  vector<string> : the list of unique reference words
 * @param  vector<map<string, double> > : the TFIDF scores
 * @return int: the id of the most similar document to the first document (having the maximum sum)
*/
int getMostSimilarDocument(map<string,int> refWords, vector<map<string, double> > tfidf);


int main()
{
    string Docs[] = {"I'd... like, an! apple.", "An apple a day keeps the doctor away.", "Never compare an apple to an orange.", "I prefer scikit-learn to orange."};
    int nDocs = (sizeof Docs / sizeof Docs[0]);

    map<string,int> refWords = getUniqueWords(Docs[0]);                          //reference words from first document

    vector<map<string, double> > tf    = computeTF(refWords, Docs, nDocs);
    map<string, double> idf            = computeIDF(refWords, Docs, nDocs);
    vector<map<string, double> > tfidf = computeTFIDF(refWords, tf, idf);

    cout << getMostSimilarDocument(refWords, tfidf);

    return 0;
}



string removePunctuation(string input)
{
    transform(input.begin(), input.end(), input.begin(), ::tolower);

    //char punctuation[] = ".,;:!?'-\"\n";
    char punctuation[] = ".,;:!?\"\n";
    for ( int i = 0; i < sizeof(punctuation); i++)
        replace(input.begin(), input.end(), punctuation[i], ' ');

    return input;
}

map<string,int> getUniqueWords(string text)
{
    string input = removePunctuation(text);
    stringstream sentence(input);

    string word;
    map<string,int> mString;

    while (getline(sentence, word, ' '))
        if ( word != "" )
        {
            if ( !mString[word])
                mString[word] = 1;
            else
                mString[word]++;
        }

    return mString;
}

vector<map<string, double> > computeTF(map<string,int> refWords, string Docs[], int nDocs)
{
    vector<map<string, double> > counts;

    for ( int i = 0; i < nDocs; i++)
    {
        map<string, double> currentCounts;
        map<string,int> wordsD = getUniqueWords(Docs[i]);

        int nWords = 0;
        for (map<string, int>::iterator it = wordsD.begin(); it != wordsD.end(); it++)
            nWords += it->second;

        for (map<string, int>::iterator it = refWords.begin(); it != refWords.end(); it++)
        {
            string rWord = it->first;

            if ( !wordsD[rWord] )
                currentCounts[rWord] = 0;
            else
                currentCounts[rWord] = wordsD[rWord];

            currentCounts[rWord] /= nWords;
        }

        counts.push_back(currentCounts);
    }

    return counts;
}

map<string, double> computeIDF(map<string,int> refWords, string Docs[], int nDocs)
{
    map<string, double> counts;

    for (map<string, int>::iterator it = refWords.begin(); it != refWords.end(); it++)
    {
        string rWord = it->first;

        double nOcc = 0;
        for (int i = 0; i < nDocs; i++ )
        {
            map<string,int> wordsD = getUniqueWords(Docs[i]);

            if ( wordsD[rWord] )
                nOcc++;
        }

        counts[rWord] = log(nDocs/nOcc);
    }

    return counts;
}

vector<map<string, double> > computeTFIDF(map<string,int> refWords, vector<map<string, double> > tf, map<string, double> idf)
{
    int const nDocs = tf.size();
    vector<map<string, double> > counts;

    for (int i = 0; i < nDocs; i++ )
    {
        map<string, double> currentCounts;

        for (map<string, int>::iterator it = refWords.begin(); it != refWords.end(); it++)
        {
            string rWord = it->first;

            if ( !tf[i][rWord] || !idf[rWord] )
                currentCounts[rWord] = 0;
            else
                currentCounts[rWord] = tf[i][rWord] * idf[rWord];
        }

        counts.push_back(currentCounts);
    }

    return counts;
}

int getMostSimilarDocument(map<string,int> refWords, vector<map<string, double> > tfidf)
{
    int const nDocs = tfidf.size();

    int maxId = 0;
    double maxSum = 0;

    for (int i = 1; i < nDocs; i++ )
    {
        double sum = 0;
        for (map<string, int>::iterator it = refWords.begin(); it != refWords.end(); it++)
            if ( tfidf[i][it->first] )
                sum += tfidf[i][it->first];

        if ( sum >= maxSum )
        {
            maxSum = sum;
            maxId  = i + 1;
        }
    }

    return maxId;
}
