#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#define MAX_TOKENS 100

typedef enum {
    NODE_NUMBER,
    NODE_VARIABLE,
    NODE_BINARY_OP,
    NODE_UNARY_MINUS
} NodeType;

struct Node {
    NodeType type;
    int num_value;      // для NODE_NUMBER
    char var_name;      // для NODE_VARIABLE
    char op;            // для NODE_BINARY_OP: '+', '-', '*', '/'
    struct Node *left;
    struct Node *right;
};
typedef struct Node Node;

typedef struct {
    Node* node;
    int odd;   // 1 если нечётное количество минусов
} SimplifyResult;


typedef enum {
    TOKEN_NUMBER,
    TOKEN_VARIABLE,
    TOKEN_OPERATOR,      // бинарные + - * /
    TOKEN_UNARY_MINUS,   // унарный минус
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_END
} TokenType;

typedef struct {
    TokenType type;
    int int_value;
    char var_name;
    char op;
} Token;

Token tokens[MAX_TOKENS];
int token_count = 0;

int is_unary_context(Token tokens[], int count) {
    if (count == 0) return 1;
    TokenType last = tokens[count-1].type;
    return (last == TOKEN_LPAREN || last == TOKEN_OPERATOR);
}

void parse_expression(const char *str) {
    int i = 0;
    while (str[i] != '\0' && token_count < MAX_TOKENS) {
        if (str[i] == ' ' || str[i] == '\t') {
            i++;
            continue;
        }

        if (str[i] >= '0' && str[i] <= '9') {
            int num = 0;
            while (str[i] >= '0' && str[i] <= '9') {
                num = num * 10 + (str[i] - '0');
                i++;
            }
            tokens[token_count].type = TOKEN_NUMBER;
            tokens[token_count].int_value = num;
            token_count++;
            continue;
        }

        if ((str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z')) {
            tokens[token_count].type = TOKEN_VARIABLE;
            tokens[token_count].var_name = str[i];
            token_count++;
            i++;
            continue;
        }

        if (str[i] == '+' || str[i] == '-' || str[i] == '*' || str[i] == '/') {
            if (str[i] == '-') {
                if (is_unary_context(tokens, token_count)) {
                    tokens[token_count].type = TOKEN_UNARY_MINUS;
                    token_count++;
                    i++;
                    continue;
                }
            }

            tokens[token_count].type = TOKEN_OPERATOR;
            tokens[token_count].op = str[i];
            token_count++;
            i++;
            continue;
        }
        if (str[i] == '(') {
            tokens[token_count].type = TOKEN_LPAREN;
            token_count++;
            i++;
            continue;
        }
        if (str[i] == ')') {
            tokens[token_count].type = TOKEN_RPAREN;
            token_count++;
            i++;
            continue;
        }

        i++;
    }
    tokens[token_count].type = TOKEN_END;
}

int priority(char op, int is_unary) {
    if (is_unary) return 3;
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

Node* create_number_node(int val) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->type = NODE_NUMBER;
    node->num_value = val;
    node->left = node->right = NULL;
    return node;
}

Node* create_variable_node(char name) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->type = NODE_VARIABLE;
    node->var_name = name;
    node->left = node->right = NULL;
    return node;
}

Node* create_binary_op(char op, Node* left, Node* right) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->type = NODE_BINARY_OP;
    node->op = op;
    node->left = left;
    node->right = right;
    return node;
}

Node* create_unary_minus(Node* operand) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->type = NODE_UNARY_MINUS;
    node->left = NULL;
    node->right = operand;
    return node;
}

Node* build_tree(Token tokens[], int token_count) {
    Node* operand_stack[MAX_TOKENS];
    int operand_top = -1;   // вершина стека операндов

    struct {
        char op;
        int is_unary;
    } operator_stack[MAX_TOKENS];
    int operator_top = -1;   // вершина стека операторов

    for (int i = 0; i < token_count && tokens[i].type != TOKEN_END; i++) {
        if (tokens[i].type == TOKEN_NUMBER) {
            operand_stack[++operand_top] = create_number_node(tokens[i].int_value);
        }
        else if (tokens[i].type == TOKEN_VARIABLE) {
            operand_stack[++operand_top] = create_variable_node(tokens[i].var_name);
        }
        else if (tokens[i].type == TOKEN_UNARY_MINUS) {
            // Кладём унарный минус в стек операторов
            operator_stack[++operator_top].op = '-';
            operator_stack[operator_top].is_unary = 1;
        }
        else if (tokens[i].type == TOKEN_OPERATOR) {
            char op = tokens[i].op;
            int prec = priority(op, 0);  // бинарный оператор
            // Выталкиваем операторы с большим или равным приоритетом
            while (operator_top >= 0 && operator_stack[operator_top].op != '(' &&
                   priority(operator_stack[operator_top].op, operator_stack[operator_top].is_unary) >= prec) {
                // Выталкиваем верхний оператор
                char top_op = operator_stack[operator_top].op;
                int is_unary = operator_stack[operator_top].is_unary;
                operator_top--;

                if (is_unary) {
                    // Унарный оператор: берём один операнд
                    Node* operand = operand_stack[operand_top--];
                    operand_stack[++operand_top] = create_unary_minus(operand);
                } else {
                    // Бинарный оператор: берём два операнда
                    Node* right = operand_stack[operand_top--];
                    Node* left  = operand_stack[operand_top--];
                    operand_stack[++operand_top] = create_binary_op(top_op, left, right);
                }
            }
            // Кладём текущий оператор в стек
            operator_stack[++operator_top].op = op;
            operator_stack[operator_top].is_unary = 0;
        }
        else if (tokens[i].type == TOKEN_LPAREN) {
            operator_stack[++operator_top].op = '(';
            operator_stack[operator_top].is_unary = 0;
        }
        else if (tokens[i].type == TOKEN_RPAREN) {
            // Выталкиваем все операторы до открывающей скобки
            while (operator_top >= 0 && operator_stack[operator_top].op != '(') {
                char top_op = operator_stack[operator_top].op;
                int is_unary = operator_stack[operator_top].is_unary;
                operator_top--;

                if (is_unary) {
                    Node* operand = operand_stack[operand_top--];
                    operand_stack[++operand_top] = create_unary_minus(operand);
                } else {
                    Node* right = operand_stack[operand_top--];
                    Node* left  = operand_stack[operand_top--];
                    operand_stack[++operand_top] = create_binary_op(top_op, left, right);
                }
            }
            // Убираем открывающую скобку
            if (operator_top >= 0 && operator_stack[operator_top].op == '(')
                operator_top--;
        }
    }

    // После обработки всех токенов выталкиваем оставшиеся операторы
    while (operator_top >= 0) {
        char top_op = operator_stack[operator_top].op;
        int is_unary = operator_stack[operator_top].is_unary;
        operator_top--;

        if (is_unary) {
            Node* operand = operand_stack[operand_top--];
            operand_stack[++operand_top] = create_unary_minus(operand);
        } else {
            Node* right = operand_stack[operand_top--];
            Node* left  = operand_stack[operand_top--];
            operand_stack[++operand_top] = create_binary_op(top_op, left, right);
        }
    }

    // В стеке операндов должен остаться один узел – корень дерева
    if (operand_top == 0)
        return operand_stack[0];
    else
        return NULL; // ошибка: выражение некорректно
}


int main() {
    char input[256];
    printf("Введите арифметическое выражение: ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = '\0';

    parse_expression(input);

    // Вывод токенов (опционально, для отладки)
    printf("\nТокены: ");
    for (int i = 0; i < token_count; i++) {
        switch (tokens[i].type) {
            case TOKEN_NUMBER: printf("NUM(%d) ", tokens[i].int_value); break;
            case TOKEN_VARIABLE: printf("VAR(%c) ", tokens[i].var_name); break;
            case TOKEN_OPERATOR: printf("OP(%c) ", tokens[i].op); break;
            case TOKEN_UNARY_MINUS: printf("UNARY_MINUS "); break;
            case TOKEN_LPAREN: printf("LP "); break;
            case TOKEN_RPAREN: printf("RP "); break;
            default: break;
        }
    }
    printf("\n");

    Node* root = build_tree(tokens, token_count);
    printf("\nИсходное выражение (текст): ");
    print_infix(root);
    printf("\nИсходное дерево:\n");
    print_tree_debug(root, 0);

    Node* simplified = simplify(root);
    printf("\nПреобразованное выражение (текст): ");
    print_infix(simplified);
    printf("\nПреобразованное дерево:\n");
    print_tree_debug(simplified, 0);

    // Освобождение памяти
    free_tree(root);
    free_tree(simplified);

    return 0;
}