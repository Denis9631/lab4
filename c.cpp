#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>  // для std::setprecision
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cmath>
#include <cctype>
#include <numeric>

std::string normalize_word(const std::string& word) {
    std::string result;
    result.reserve(word.size());
    
    std::transform(word.begin(), word.end(), std::back_inserter(result),
                   [](unsigned char c) { return std::tolower(c); });
    
    result.erase(std::remove_if(result.begin(), result.end(), 
                                [](unsigned char c) { return std::ispunct(c) && c!='-'; }), 
                 result.end());
    
    return result;
}

std::vector<std::string> split_into_words(const std::string& text) {
    std::vector<std::string> words;
    std::istringstream iss(text);
    
    std::transform(std::istream_iterator<std::string>(iss),
                   std::istream_iterator<std::string>(),
                   std::back_inserter(words),
                   [](const std::string& word) { return normalize_word(word); });
    
    return words;
}

std::vector<std::string> read_file_list(const std::string& list_file) {
    std::vector<std::string> files;
    std::ifstream file(list_file);
    
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open list file: " << list_file << std::endl;
        exit(1);
    }
    
    std::copy(std::istream_iterator<std::string>(file),
              std::istream_iterator<std::string>(),
              std::back_inserter(files));
    
    return files;
}

std::string read_file_content(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file: " << filename << std::endl;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Подсчет количества вхождений слова в документе
int count_word_in_document(const std::string& word, const std::string& document_name) {
    std::string content = read_file_content(document_name);
    if (content.empty()) return 0;
    
    std::vector<std::string> words = split_into_words(content);
    
    return std::count_if(words.begin(), words.end(),
                         [&word](const std::string& w) { return w == word; });
}

// Подсчет общего количества слов в документе
int count_total_words_in_document(const std::string& document_name) {
    std::string content = read_file_content(document_name);
    if (content.empty()) return 0;
    
    std::vector<std::string> words = split_into_words(content);
    return words.size();
}

// Вычисление TF (Term Frequency)
double calculate_tf(const std::string& word, const std::string& document_name) {
    int word_count = count_word_in_document(word, document_name);
    int total_words = count_total_words_in_document(document_name);
    
    if (total_words == 0) return 0.0;
    return static_cast<double>(word_count) / total_words;
}

// Подсчет количества документов, содержащих слово
int count_documents_with_word(const std::string& word, 
                              const std::vector<std::string>& file_list) {
    int count = 0;
    
    std::for_each(file_list.begin(), file_list.end(),
                  [&](const std::string& filename) {
                      int word_count = count_word_in_document(word, filename);
                      if (word_count > 0) count++;
                  });
    
    return count;
}

// Вычисление IDF (Inverse Document Frequency)
double calculate_idf(const std::string& word, 
                     const std::vector<std::string>& file_list) {
    int total_docs = file_list.size();
    int docs_with_word = count_documents_with_word(word, file_list);
    
    if (docs_with_word == 0) return 0.0;
    return std::log(static_cast<double>(total_docs) / docs_with_word);
}

// Вычисление TF-IDF
double calculate_tf_idf(const std::string& word, 
                        const std::string& document_name,
                        const std::vector<std::string>& file_list) {
    double tf = calculate_tf(word, document_name);
    double idf = calculate_idf(word, file_list);
    
    return tf * idf;
}

// Получение множества документов, содержащих слово
std::set<std::string> get_documents_with_word(const std::string& word, 
                                               const std::vector<std::string>& file_list) {
    std::set<std::string> documents;
    
    std::for_each(file_list.begin(), file_list.end(),
                  [&](const std::string& filename) {
                      int word_count = count_word_in_document(word, filename);
                      if (word_count > 0) {
                          documents.insert(filename);
                      }
                  });
    
    return documents;
}

// Обработка команды WORD
void handle_word_command(const std::string& search_word, 
                         const std::vector<std::string>& file_list) {
    
    std::string normalized_search = normalize_word(search_word);
    int total_documents = file_list.size();
    
    std::set<std::string> documents = get_documents_with_word(normalized_search, file_list);
    double idf = calculate_idf(normalized_search, file_list);
    
    std::cout << "Word : " << search_word << std::endl;
    std::cout << "Documents total : " << total_documents << std::endl;
    std::cout << "Documents with word : " << documents.size() << std::endl;
    std::cout << std::fixed;
    std::cout.precision(4);
    std::cout << "IDF : " << idf << std::endl;
    std::cout << "Appears in :" << std::endl;
    
    if (!documents.empty()) {
        std::for_each(documents.begin(), documents.end(),
                      [](const std::string& doc) {
                          std::cout << "- " << doc << std::endl;
                      });
    } else {
        std::cout << "(none)" << std::endl;
    }
}

// Обработка команды WORD_IN_DOC
void handle_word_in_doc_command(const std::string& search_word, 
                                const std::string& document_name,
                                const std::vector<std::string>& file_list) {
    
    std::string normalized_search = normalize_word(search_word);
    
    auto doc_it = std::find(file_list.begin(), file_list.end(), document_name);
    if (doc_it == file_list.end()) {
        std::cerr << "Error: Document not found in list: " << document_name << std::endl;
        return;
    }
    
    int word_count = count_word_in_document(normalized_search, document_name);
    double tf = calculate_tf(normalized_search, document_name);
    double tf_idf = calculate_tf_idf(normalized_search, document_name, file_list);
    
    std::cout << "Word : " << search_word << std::endl;
    std::cout << "Document : " << document_name << std::endl;
    std::cout << "Count : " << word_count << std::endl;
    std::cout << std::fixed;
    std::cout.precision(4);
    std::cout << "TF : " << tf << std::endl;
    std::cout << "TF-IDF : " << tf_idf << std::endl;
}

// Обработка команды DOC
void handle_doc_command(const std::string& document_name,
                        const std::vector<std::string>& file_list) {
    
    auto doc_it = std::find(file_list.begin(), file_list.end(), document_name);
    if (doc_it == file_list.end()) {
        std::cerr << "Error: Document not found in list: " << document_name << std::endl;
        return;
    }
    
    std::string content = read_file_content(document_name);
    if (content.empty()) {
        std::cerr << "Error: Cannot read document: " << document_name << std::endl;
        return;
    }
    
    std::vector<std::string> words = split_into_words(content);
    int total_words = words.size();
    
    std::map<std::string, int> word_freq;
    std::for_each(words.begin(), words.end(),
                  [&word_freq](const std::string& word) {
                      word_freq[word]++;
                  });
    
    int unique_words = word_freq.size();
    
    std::vector<std::pair<std::string, double>> word_tf;
    std::transform(word_freq.begin(), word_freq.end(),
                   std::back_inserter(word_tf),
                   [total_words](const auto& pair) {
                       double tf = static_cast<double>(pair.second) / total_words;
                       return std::make_pair(pair.first, tf);
                   });
    
    std::sort(word_tf.begin(), word_tf.end(),
              [](const auto& a, const auto& b) {
                  return a.second > b.second;
              });
    
    std::cout << "Document : " << document_name << std::endl;
    std::cout << "Total words : " << total_words << std::endl;
    std::cout << "Unique words : " << unique_words << std::endl;
    std::cout << "Top words :" << std::endl;
    
    int count = 0;
    std::for_each(word_tf.begin(), word_tf.end(),
                  [&count](const auto& pair) {
                      if (count >= 5) return;
                      std::cout << ++count << ". " << pair.first 
                               << " (" << std::fixed << std::setprecision(4) 
                               << pair.second << ")" << std::endl;
                  });
}

// Вычисление TF-IDF для слова во всех документах
std::vector<std::pair<std::string, double>> get_word_tf_idf_scores(
    const std::string& word, 
    const std::vector<std::string>& file_list) {
    
    std::vector<std::pair<std::string, double>> scores;
    
    std::transform(file_list.begin(), file_list.end(),
                   std::back_inserter(scores),
                   [&word, &file_list](const std::string& doc_name) {
                       double tf_idf = calculate_tf_idf(word, doc_name, file_list);
                       return std::make_pair(doc_name, tf_idf);
                   });
    
    return scores;
}

// Обработка команды QUERY
void handle_query_command(const std::vector<std::string>& query_words,
                          const std::vector<std::string>& file_list) {
    
    std::vector<std::string> normalized_query;
    std::transform(query_words.begin(), query_words.end(),
                   std::back_inserter(normalized_query),
                   [](const std::string& word) { return normalize_word(word); });
    
    std::map<std::string, double> doc_scores;
    
    std::for_each(normalized_query.begin(), normalized_query.end(),
                  [&](const std::string& query_word) {
                      auto word_scores = get_word_tf_idf_scores(query_word, file_list);
                      
                      std::for_each(word_scores.begin(), word_scores.end(),
                                    [&](const auto& pair) {
                                        doc_scores[pair.first] += pair.second;
                                    });
                  });
    
    std::vector<std::pair<std::string, double>> ranked_docs;
    std::copy(doc_scores.begin(), doc_scores.end(),
              std::back_inserter(ranked_docs));
    
    ranked_docs.erase(
        std::remove_if(ranked_docs.begin(), ranked_docs.end(),
                       [](const auto& pair) { return pair.second == 0.0; }),
        ranked_docs.end()
    );
    
    std::sort(ranked_docs.begin(), ranked_docs.end(),
              [](const auto& a, const auto& b) {
                  return a.second > b.second;
              });
    
    std::cout << "Query :";
    std::for_each(query_words.begin(), query_words.end(),
                  [](const std::string& word) {
                      std::cout << " " << word;
                  });
    std::cout << std::endl;
    
    std::cout << "Results :" << std::endl;
    
    if (!ranked_docs.empty()) {
        int count = 0;
        std::for_each(ranked_docs.begin(), ranked_docs.end(),
                      [&count](const auto& pair) {
                          std::cout << ++count << ". " << pair.first 
                                   << " (" << std::fixed << std::setprecision(4) 
                                   << pair.second << ")" << std::endl;
                      });
    } else {
        std::cout << "(no matching documents)" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::string list_file = "documents.txt";
    std::vector<std::string> file_list = read_file_list(list_file);
    std::string input_line;
    while (std::getline(std::cin, input_line)) {
        if (input_line.empty()) {
            std::cout << "Enter command: ";
            continue;
        }
        
        std::istringstream iss(input_line);
        std::vector<std::string> args;
        std::copy(std::istream_iterator<std::string>(iss),
                  std::istream_iterator<std::string>(),
                  std::back_inserter(args));
        
        if (args.empty()) {
            std::cout << "Enter command: ";
            continue;
        }
        
        std::string command = args[0];
        
        if (command == "EXIT") {
            std::cout << "End of program" << std::endl;
            break;
        }
        if (command == "WORD" && args.size() == 2) {
            handle_word_command(args[1], file_list);
        }
        else if (command == "WORD_IN_DOC" && args.size() == 3) {
            handle_word_in_doc_command(args[1], args[2], file_list);
        }
        else if (command == "DOC" && args.size() == 2) {
            handle_doc_command(args[1], file_list);
        }
        else if (command == "QUERY" && args.size() >= 2) {
            std::vector<std::string> query_words(args.begin() + 1, args.end());
            handle_query_command(query_words, file_list);
        }
        else {
            std::cerr << "Error: Invalid command or arguments" << std::endl;
        }
    }
    return 0;
}