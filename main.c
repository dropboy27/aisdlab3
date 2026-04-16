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

typedef struct Node {
    NodeType type;
    int num_value;
    char var_name;
    char op;
    struct Node *left;
    struct Node *right;
}Node;

typedef struct {
    Node* node;
    int odd;   //если нечётное количество минусов
} SimplifyResult;


typedef enum {
    TOKEN_NUMBER,
    TOKEN_VARIABLE,
    TOKEN_OPERATOR,      // бинарные
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

int is_unary(Token tokens[], int count) {
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
                if (is_unary(tokens, token_count)) {
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

Node* build_tree(Token *tokens, int length){
    Node* operand_stack[MAX_TOKENS];
    int operand_stack_len = -1;
    struct {
        char op;
        int is_unary;
    } operator_stack[MAX_TOKENS];
    int operator_stack_len = -1;
    for (int i = 0; i < length; i++){
        Token current_element = tokens[i];
        if (current_element.type == TOKEN_NUMBER){
            Node *New_Node = create_number_node(current_element.int_value);
            operand_stack[++operand_stack_len] = New_Node;
            continue;
        }
        else if (current_element.type == TOKEN_VARIABLE){
            Node *New_Node = create_variable_node(current_element.var_name);
            operand_stack[++operand_stack_len] = New_Node;
            continue;
        }
        else if (current_element.type == TOKEN_UNARY_MINUS){
            operator_stack[++operator_stack_len].op = '-';
            operator_stack[operator_stack_len].is_unary = 1;
            continue;
        }
        else if (current_element.type == TOKEN_OPERATOR){
            while (operator_stack_len >= 0
                            && priority(operator_stack[operator_stack_len].op,
                            operator_stack[operator_stack_len].is_unary)>=priority(current_element.op, 0)
                            && operator_stack[operator_stack_len].op!='(')
            {
                char op = operator_stack[operator_stack_len].op;
                int is_unary = operator_stack[operator_stack_len].is_unary;
                operator_stack_len--;
                if (is_unary) {
                    Node* operand = operand_stack[operand_stack_len];
                    operand_stack[operand_stack_len] = create_unary_minus(operand);
                } else {
                    Node* right = operand_stack[operand_stack_len--];
                    Node* left  = operand_stack[operand_stack_len];
                    Node* new_node = create_binary_op(op, left, right);
                    operand_stack[operand_stack_len] = new_node;
                }
            }
            operator_stack[++operator_stack_len].op = current_element.op;
            operator_stack[operator_stack_len].is_unary = 0;
            continue;
        }
        else if (current_element.type == TOKEN_LPAREN){
            operator_stack[++operator_stack_len].op = '(';
            operator_stack[operator_stack_len].is_unary = 0;
            continue;
        }
        else if (current_element.type == TOKEN_RPAREN){
            while (operator_stack_len >= 0 && operator_stack[operator_stack_len].op != '('){
                char op = operator_stack[operator_stack_len].op;
                int is_unary = operator_stack[operator_stack_len].is_unary;
                operator_stack_len--;
                if (is_unary) {
                    Node* operand = operand_stack[operand_stack_len];
                    operand_stack[operand_stack_len] = create_unary_minus(operand);
                } else {
                    Node* right = operand_stack[operand_stack_len--];
                    Node* left  = operand_stack[operand_stack_len];
                    Node* new_node = create_binary_op(op, left, right);
                    operand_stack[operand_stack_len] = new_node;
                }
            }
            if (operator_stack_len >= 0 && operator_stack[operator_stack_len].op == '(')
                operator_stack_len--;
            continue;
        }
    }
    while (operator_stack_len>=0){
        char op = operator_stack[operator_stack_len].op;
        int is_unary = operator_stack[operator_stack_len].is_unary;
        operator_stack_len--;
        if (is_unary) {
            Node* operand = operand_stack[operand_stack_len];
            operand_stack[operand_stack_len] = create_unary_minus(operand);
        } else {
            Node* right = operand_stack[operand_stack_len--];
            Node* left  = operand_stack[operand_stack_len];
            Node* new_node = create_binary_op(op, left, right);
            operand_stack[operand_stack_len] = new_node;
        }
    }
    if (operand_stack_len == 0) {
        return operand_stack[0];
    } else {
        // Ошибка: в стеке операндов не один элемент
        printf("Ошибка: некорректное выражение\n");
        return NULL;
    }
}


void print_tree_vertical(Node* node, int level) {
    if (!node) return;

    // Сначала печатаем правого потомка
    if (node->type == NODE_BINARY_OP || node->type == NODE_UNARY_MINUS) {
        print_tree_vertical(node->right, level + 1);
    }

    // Отступы
    for (int i = 0; i < level; i++)
        printf("    ");

    // Печатаем текущий узел
    if (node->type == NODE_NUMBER)
        printf("%d\n", node->num_value);
    else if (node->type == NODE_VARIABLE)
        printf("%c\n", node->var_name);
    else if (node->type == NODE_BINARY_OP)
        printf("%c\n", node->op);
    else if (node->type == NODE_UNARY_MINUS)
        // можно вывести как "-" или "(-)"
        printf("-\n");

    // Затем левого потомка (будет ниже)
    if (node->type == NODE_BINARY_OP) {
        print_tree_vertical(node->left, level + 1);
    }
}


void print_infix(Node* node) {
    if (!node) return;

    if (node->type == NODE_NUMBER){
        printf("%d", node->num_value);
    }
    else if (node->type == NODE_VARIABLE){
        printf("%c", node->var_name);
    }
    else if (node->type == NODE_UNARY_MINUS){
        printf("-");
        if (node->right->type != NODE_NUMBER && node->right->type != NODE_VARIABLE){
            printf("(");
            print_infix(node->right);
            printf(")");
        }
        else{
            print_infix(node->right);
        }
    }
    else if (node->type == NODE_BINARY_OP){
        if (node->left->type == NODE_BINARY_OP){
            if (priority(node->op, 0)>priority(node->left->op, 0)){
                printf("(");
                print_infix(node->left);
                printf(")");
            }
            else {
                print_infix(node->left);
            }
        }
        else{
            print_infix(node->left);
        }
        printf(" %c ", node->op);
        if (node->right->type == NODE_BINARY_OP){
            if (priority(node->op, 0)>priority(node->right->op, 0) ||
            (priority(node->op, 0) == priority(node->right->op, 0) &&
            (node->right->op == '/' || node->right->op == '-'))){
                printf("(");
                print_infix(node->right);
                printf(")");
                }
            else {
                print_infix(node->right);
            }
        }
        else if (node->right->type == NODE_UNARY_MINUS && (node->op == '-' || node->op == '/')){
            printf("(");
            print_infix(node->right);
            printf(")");
        }
        else{
            print_infix(node->right);
        }
    }
}

Node* copy_node(Node* src) {
    Node* dst = (Node*)malloc(sizeof(Node));
    dst->type = src->type;
    if (src->type == NODE_NUMBER){
        dst->num_value = src->num_value;
    }
    else if (src->type == NODE_VARIABLE){
        dst->var_name = src->var_name;
    }
    dst->left = dst->right = NULL;
    return dst;
}

// Рекурсивное упрощение (вынос унарных минусов)
SimplifyResult extract_minuses(Node* node) {
    SimplifyResult res;
    if (!node){
        res.node = NULL;
        res.odd = 0;
        return res;
    }

    if (node->type == NODE_NUMBER || node->type == NODE_VARIABLE) {
        res.node = copy_node(node);
        res.odd = 0;
        return res;
    }
    if (node->type == NODE_UNARY_MINUS) {
        SimplifyResult child = extract_minuses(node->right);
        res.node = child.node;
        res.odd = 1 - child.odd;
        return res;
    }
    if (node->type == NODE_BINARY_OP && node->op == '*') {
        SimplifyResult left_res = extract_minuses(node->left);
        SimplifyResult right_res = extract_minuses(node->right);
        int total_odd = (left_res.odd + right_res.odd) % 2;
        Node* prod = create_binary_op('*', left_res.node, right_res.node);
        res.node = prod;
        res.odd = total_odd;
        return res;
    }
    // Для других операций обходим, минусы не выносим
    SimplifyResult left_res = extract_minuses(node->left);
    SimplifyResult right_res = extract_minuses(node->right);
    res.node = create_binary_op(node->op, left_res.node, right_res.node);
    res.odd = 0;
    return res;
}

Node* simplify(Node* root) {
    SimplifyResult res = extract_minuses(root);
    if (res.odd == 1)
        return create_unary_minus(res.node);
    else
        return res.node;
}

// Освобождение памяти дерева
void free_tree(Node* node) {
    if (!node) return;
    if (node->type == NODE_BINARY_OP) {
        free_tree(node->left);
        free_tree(node->right);
    } else if (node->type == NODE_UNARY_MINUS) {
        free_tree(node->right);
    }
    free(node);
}

int main() {
    char input[256];
    printf("Enter expression: ");
    fgets(input, sizeof(input), stdin);
    input[strcspn(input, "\n")] = '\0';

    // 1. Исходное выражение в текстовом виде
    printf("Original expression: %s\n", input);

    parse_expression(input);
    Node* root = build_tree(tokens, token_count);
    if (!root) {
        printf("Error building tree\n");
        return 1;
    }

    Node* simplified = simplify(root);

    // 2. Преобразованное дерево
    printf("\nTransformated tree:\n");
    print_tree_vertical(simplified, 0);

    // 3. Преобразованное выражение в текстовом виде
    printf("\nTransformated expression: ");
    print_infix(simplified);
    printf("\n");

    free_tree(root);
    free_tree(simplified);
    return 0;
}


