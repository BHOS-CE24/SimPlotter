#include <cmath>
#include <string>
#include <cstdint>
#include <SDL2/SDL.h>
#include <stack>
#include <iostream>
#include <algorithm>
#include <cstdint>
#include <ctime>
#include <limits>

#define E  2.71828182845904523536
#define PI 3.14159265358979323846

namespace Operation {

float applyOp(float a, float b, char op) {
    switch (op) {
    case '+': return a + b;
    case '-': return a - b;
    case '*': return a * b;
    case '/':
        if (b == 0.0f) return NAN;
        return a / b;
    case '%':
        if (b == 0.0f) return NAN;
        return fmodf(a, b);          
    case '^': return powf(a, b);
    }
    return 0;
}

} // namespace Operation

struct Camera {
    float Xoffset = 0.0f;
    float Yoffset = 0.0f;
    float range = 50.0f;
};

struct State {
    bool moved = false;
    int dragStartX = 0;
    int dragStartY = 0;
    float camPositionX = 0.0f;
    float camPositionY = 0.0f;
};


float applyFunction(const std::string& func, float value) {
    if (func == "sqrt") return sqrtf(value);
    if (func == "sin") return sinf(value);
    if (func == "cos") return cosf(value);
    if (func == "tan") return tanf(value);
    if (func == "asin") return asinf(value);
    if (func == "acos") return acosf(value);
    if (func == "atan") return atanf(value);
    if (func == "sinh") return sinhf(value);
    if (func == "cosh") return coshf(value);
    if (func == "tanh") return tanhf(value);
    if (func == "log") return log10f(value);
    if (func == "ln") return logf(value);
    if (func == "exp") return expf(value);
    if (func == "abs") return fabsf(value);
    if (func == "ceil") return ceilf(value);
    if (func == "floor") return floorf(value);
    if (func == "round") return roundf(value);
    return value;
}

int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/' || op == '%') return 2;
    if (op == '^') return 3;
    if (op == 'u') return 4; 
    return 0;
}

bool isRightAssociative(char op) {
    return (op == '^' || op == 'u');  
}


bool isFunction(const std::string& expr, int pos, const std::string& func) {
    int len = (int)func.length();
    if (pos + len > (int)expr.length()) return false;
    std::string sub = expr.substr(pos, len);
    std::transform(sub.begin(), sub.end(), sub.begin(), ::tolower);
    if (sub != func) return false;
    if (pos + len < (int)expr.length() && isalpha(expr[pos + len])) return false;
    return true;
}

std::string getFunctionName(const std::string& expr, int pos) {
    static const std::string functions[] = {
        "sqrt", "asin", "acos", "atan", "sinh", "cosh", "tanh",
        "sin",  "cos",  "tan",  "log",  "ln",   "exp",
        "abs",  "ceil", "floor","round"
    };
    static const int N = 16;
    for (int i = 0; i < N; i++) {
        if (isFunction(expr, pos, functions[i])) return functions[i];
    }
    return "";
}

template<typename D>
D factorial(D a) {
    if (a < D(0)) return D(NAN);   
    int n = (int)roundf((float)a);
    if (n > 20) return D(INFINITY);
    D f = D(1);
    for (int i = 2; i <= n; ++i) f *= D(i);
    return f;
}

template<typename A, typename B>
bool applyTop(std::stack<A>& values, std::stack<B>& ops) {
    if (ops.empty()) return false;

    char op = ops.top();

    if (op == 'u') {
        if (values.empty()) return false;
        ops.pop();
        float val = values.top(); values.pop();
        values.push(-val);
        return true;
    }

    if (values.size() < 2) return false;
    ops.pop();
    float b = values.top(); values.pop();
    float a = values.top(); values.pop();
    values.push(Operation::applyOp(a, b, op));
    return true;
}


bool evaluateWithX(const std::string& expression, float xValue, float& result) {

    std::string expr;
    expr.reserve(expression.size() * 2);
    for (int i = 0; i < (int)expression.size(); i++) {
        if (expression[i] == 'x') {
            bool standalone = true;
            if (i > 0 && (isalpha(expression[i-1]) || isdigit(expression[i-1])))
                standalone = false;
            if (i < (int)expression.size() - 1 &&
                (isalpha(expression[i+1]) || isdigit(expression[i+1])))
                standalone = false;
            if (standalone) {
                char buf[32];
                snprintf(buf, sizeof(buf), "(%.10f)", xValue);
                expr += buf;
            } else {
                expr += expression[i];
            }
        } else {
            expr += expression[i];
        }
    }

    expr.erase(std::remove(expr.begin(), expr.end(), ' '), expr.end());

    {
        std::string piStr = std::to_string(PI);
        size_t pos = 0;
        while ((pos = expr.find("pi", pos)) != std::string::npos) {
            bool standalone = true;
            if (pos > 0 && (isalpha(expr[pos-1]) || isdigit(expr[pos-1])))
                standalone = false;
            if (pos + 2 < expr.size() && (isalpha(expr[pos+2]) || isdigit(expr[pos+2])))
                standalone = false;
            if (standalone) {
                expr.replace(pos, 2, piStr);
                pos += piStr.length();
            } else {
                pos += 2;
            }
        }
    }


    {
        std::string eStr = std::to_string(E);
        size_t pos = 0;
        while (pos < expr.size()) {
            size_t found = expr.find('e', pos);
            if (found == std::string::npos) break;

            bool standalone = true;
            if (found > 0 && isalpha(expr[found - 1])) standalone = false;
            if (found + 1 < expr.size() && isalpha(expr[found + 1])) standalone = false;
            if (found > 0 && isdigit(expr[found - 1])) standalone = false;

            if (standalone) {
                expr.replace(found, 1, eStr);
                pos = found + eStr.length();
            } else {
                pos = found + 1;
            }
        }
    }

    if (expr.empty()) return false;


    {
        std::string tmp;
        tmp.reserve(expr.size() * 2);
        for (int i = 0; i < (int)expr.size(); i++) {
            tmp += expr[i];
            if (i + 1 < (int)expr.size()) {
                char cur  = expr[i];
                char next = expr[i + 1];
                bool curIsNum   = isdigit(cur) || cur  == ')';
                bool nextIsNum  = isdigit(next) || next == '(';
                if (curIsNum && next == '(') tmp += '*';
                else if (cur == ')' && isdigit(next)) tmp += '*';
                else if (cur == ')' && next == '(') tmp += '*';
                else if (isdigit(cur) && isalpha(next)) tmp += '*';
            }
        }
        expr = tmp;
    }

    std::stack<float> values;
    std::stack<char> ops;
    std::stack<std::string> funcs;

    int n = (int)expr.size();
    for (int i = 0; i < n; i++) {


        std::string funcName = getFunctionName(expr, i);
        if (!funcName.empty()) {
            i += (int)funcName.length() - 1;
            funcs.push(funcName);
            continue;
        }

        if (expr[i] == '(') {
            ops.push('(');
        }
        else if (expr[i] == ')') {
            while (!ops.empty() && ops.top() != '(') {
                if (!applyTop(values, ops)) return false;
            }
            if (ops.empty()) return false;  
            ops.pop(); 

            if (!funcs.empty()) {
                if (values.empty()) return false;
                float val = values.top(); values.pop();
                values.push(applyFunction(funcs.top(), val));
                funcs.pop();
            }
        }
        else if (isdigit(expr[i]) || expr[i] == '.') {
            float num = 0.0f;
            float decimal = 0.0f;
            bool  hasDecimal   = false;
            int   decimalPlaces = 0;

            while (i < n && (isdigit(expr[i]) || expr[i] == '.')) {
                if (expr[i] == '.') {
                    if (hasDecimal) return false;
                    hasDecimal = true;
                } else {
                    if (hasDecimal) {
                        decimal = decimal * 10.0f + (expr[i] - '0');
                        decimalPlaces++;
                    } else {
                        num = num * 10.0f + (expr[i] - '0');
                    }
                }
                i++;
            }
            i--; 

            if (hasDecimal && decimalPlaces > 0)
                num += decimal / powf(10.0f, (float)decimalPlaces);

            values.push(num);
        }
        else if (expr[i] == '-' &&
                 (i == 0 || expr[i-1] == '(' ||
                  expr[i-1] == '+' || expr[i-1] == '-' ||
                  expr[i-1] == '*' || expr[i-1] == '/' ||
                  expr[i-1] == '^' || expr[i-1] == '%')) {
            while (!ops.empty() && ops.top() != '(' &&
                   (precedence(ops.top()) > precedence('u') ||
                    (precedence(ops.top()) == precedence('u') && !isRightAssociative('u')))) {
                if (!applyTop(values, ops)) return false;
            }
            ops.push('u');
        }
        else if (expr[i] == '+' || expr[i] == '-' ||
                 expr[i] == '*' || expr[i] == '/'  ||
                 expr[i] == '%' || expr[i] == '^') {
            while (!ops.empty() && ops.top() != '(' &&
                   (precedence(ops.top()) > precedence(expr[i]) ||
                    (precedence(ops.top()) == precedence(expr[i]) &&
                     !isRightAssociative(expr[i])))) {
                if (!applyTop(values, ops)) return false;
            }
            ops.push(expr[i]);
        }
        else if (expr[i] == '!') {
            if (values.empty()) return false;
            float val = values.top(); values.pop();
            values.push(factorial(val));
        }
        else {
            return false; 
        }
    }

    while (!ops.empty()) {
        if (ops.top() == '(') return false; 
        if (!applyTop(values, ops)) return false;
    }

    if (values.size() != 1) return false;
    result = values.top();
    return true;
}


static void setColour(SDL_Renderer* renderer, const std::string& lineColour) {
    struct ColEntry { const char* name; Uint8 r, g, b; };
    static const ColEntry table[] = {
        {"red", 255, 0, 0},
        {"green", 0, 255, 0},
        {"blue", 0, 0, 255},
        {"black", 0, 0, 0},
        {"yellow", 255, 255, 0},
        {"cyan", 0, 255, 255},
        {"magenta", 255, 0, 255},
        {"orange", 255, 160, 0},
        {"purple", 128, 0, 255},
    };
    for (auto& e : table) {
        if (lineColour == e.name) {
            SDL_SetRenderDrawColor(renderer, e.r, e.g, e.b, 255);
            return;
        }
    }
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); 
}

static void drawGrid(SDL_Renderer* renderer, int w, int h, Camera& cam) {
    float pixelsPerUnit = (float)w / cam.range;
    int centerX = (int)roundf(w / 2.0f - cam.Xoffset * pixelsPerUnit);
    int centerY = (int)roundf(h / 2.0f + cam.Yoffset * pixelsPerUnit);
    float half = cam.range / 2.0f;
    float xMin = cam.Xoffset - half;
    float xMax = cam.Xoffset + half;

    for (float gx = floorf(xMin); gx <= xMax; gx += 1.0f) {
        int px = centerX + (int)(gx * pixelsPerUnit);
        if (px < 0 || px >= w) continue;
        if (fabsf(gx) < 0.001f) SDL_SetRenderDrawColor(renderer, 190, 190, 190, 255);
        else SDL_SetRenderDrawColor(renderer, 210, 210, 210, 255);
        SDL_RenderDrawLine(renderer, px, 0, px, h);
    }
    float yMin = cam.Yoffset - half;
    float yMax = cam.Yoffset + half;
    for (float gy = floorf(yMin); gy <= yMax; gy += 1.0f) {
        int py = centerY - (int)(gy * pixelsPerUnit);
        if (py < 0 || py >= h) continue;
        if (fabsf(gy) < 0.001f) SDL_SetRenderDrawColor(renderer, 190, 190, 190, 255);
        else SDL_SetRenderDrawColor(renderer, 210, 210, 210, 255);
        SDL_RenderDrawLine(renderer, 0, py, w, py);
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawLine(renderer, 0, centerY, w, centerY);
    SDL_RenderDrawLine(renderer, 0, centerY + 1, w, centerY + 1);
    SDL_RenderDrawLine(renderer, centerX, 0, centerX, h);
    SDL_RenderDrawLine(renderer, centerX + 1, 0, centerX + 1, h);
}


void renderAction(SDL_Renderer* renderer, const std::string& expression,
                  const std::string& lineColour, int w, int h, Camera& cam) {

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    float pixelsPerUnit = (float)w / cam.range;
    int centerX = (int)roundf(w / 2.0f - cam.Xoffset * pixelsPerUnit);
    int centerY = (int)roundf(h / 2.0f + cam.Yoffset * pixelsPerUnit);

    drawGrid(renderer, w, h, cam);
    setColour(renderer, lineColour);

    float prevY = NAN;
    int prevPx = -1, prevPy = -1;

    for (int px = 0; px < w; px++) {
        float mathX = (px - centerX) / pixelsPerUnit;
        float mathY;
        if (!evaluateWithX(expression, mathX, mathY) || isnan(mathY) || isinf(mathY)) {
            prevY = NAN;
            continue;
        }

        int py = centerY - (int)(mathY * pixelsPerUnit);
        if (py >= 0 && py < h) {
            if (!isnan(prevY) && prevPx >= 0 && abs(py - prevPy) < h / 2) {
                SDL_RenderDrawLine(renderer, prevPx, prevPy, px, py);
            } else {
                SDL_RenderDrawPoint(renderer, px, py);
            }
            prevY  = mathY;
            prevPx = px;
            prevPy = py;
        } else {
            prevY = NAN;
        }
    }

    SDL_RenderPresent(renderer);
}


void renderParametric(SDL_Renderer* renderer, const std::string& exprX,
                      const std::string& exprY, const std::string& lineColour,
                      int w, int h, Camera& cam) {

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    float pixelsPerUnit = (float)w / cam.range;
    int centerX = (int)roundf(w / 2.0f - cam.Xoffset * pixelsPerUnit);
    int centerY = (int)roundf(h / 2.0f + cam.Yoffset * pixelsPerUnit);

    drawGrid(renderer, w, h, cam);
    setColour(renderer, lineColour);

    const int   steps = 10000;
    const float tMin = 0.0f;
    const float tMax = 2.0f * (float)PI;
    const float dt = (tMax - tMin) / steps;

    int  prevPx = -1, prevPy = -1;
    bool hasPrev = false;

    for (int i = 0; i <= steps; i++) {
        float t = tMin + i * dt;
        float mathX, mathY;

        if (!evaluateWithX(exprX, t, mathX) || !evaluateWithX(exprY, t, mathY) ||
            isnan(mathX) || isinf(mathX) || isnan(mathY) || isinf(mathY)) {
            hasPrev = false;
            continue;
        }

        int px = centerX + (int)(mathX * pixelsPerUnit);
        int py = centerY - (int)(mathY * pixelsPerUnit);

        if (px >= 0 && px < w && py >= 0 && py < h) {
            if (hasPrev) SDL_RenderDrawLine(renderer, prevPx, prevPy, px, py);
            else SDL_RenderDrawPoint(renderer, px, py);
            prevPx = px; prevPy = py; hasPrev = true;
        } else {
            hasPrev = false;
        }
    }

    SDL_RenderPresent(renderer);
}

static void handleEvents(SDL_Event& e, Camera& cam, State& input, bool& quit, int w, int h) {
    float pixel = (float)w / cam.range;
    float step  = cam.range / 20.0f;

    if (e.type == SDL_QUIT) { quit = true; return; }
    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) { quit = true; return; }

    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
        case SDLK_LEFT: cam.Xoffset -= step; break;
        case SDLK_RIGHT: cam.Xoffset += step; break;
        case SDLK_UP: cam.Yoffset += step; break;
        case SDLK_DOWN: cam.Yoffset -= step; break;
        case SDLK_HOME: cam.Xoffset = 0; cam.Yoffset = 0; cam.range = 50.0f; break;
        default: break;
        }
    }
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_MIDDLE) {
        cam.Xoffset = 0; cam.Yoffset = 0; cam.range = 50.0f;
    }
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        input.moved = true;
        input.dragStartX = e.button.x;
        input.dragStartY = e.button.y;
        input.camPositionX = cam.Xoffset;
        input.camPositionY = cam.Yoffset;
    }
    if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT)
        input.moved = false;

    if (e.type == SDL_MOUSEMOTION && input.moved) {
        int dx = e.motion.x - input.dragStartX;
        int dy = e.motion.y - input.dragStartY;
        cam.Xoffset = input.camPositionX - dx / pixel;
        cam.Yoffset = input.camPositionY + dy / pixel;
    }
    if (e.type == SDL_MOUSEWHEEL) {
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        float originX = w / 2.0f - cam.Xoffset * pixel;
        float originY = h / 2.0f + cam.Yoffset * pixel;
        float mathX = (mouseX - originX) / pixel;
        float mathY = (originY - mouseY) / pixel;
        float zf = (e.wheel.y > 0) ? 0.9f : 1.1f;
        cam.range *= zf;
        cam.Xoffset += mathX * (1.0f - zf);
        cam.Yoffset += mathY * (1.0f - zf);
    }
}


void funcToGraph(const std::string& expression, const std::string& lineColour, Camera& cam) {
    SDL_Window* window = SDL_CreateWindow("Simple Plotter",
                                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                 1280, 720, SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Event e;
    bool quit = false;
    State input;
    int w = 1280, h = 720;

    while (!quit) {
        SDL_GetWindowSize(window, &w, &h);
        while (SDL_PollEvent(&e)) handleEvents(e, cam, input, quit, w, h);
        SDL_GetWindowSize(window, &w, &h);
        renderAction(renderer, expression, lineColour, w, h, cam);
        SDL_Delay(10);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

void funcToGraphParametric(const std::string& expressionX, const std::string& expressionY,
                            const std::string& lineColour, Camera& cam) {
    SDL_Window* window = SDL_CreateWindow("Simple Plotter",
                                 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                 1280, 720, SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Event e;
    bool quit = false;
    State input;
    int w = 1280, h = 720;

    while (!quit) {
        SDL_GetWindowSize(window, &w, &h);
        while (SDL_PollEvent(&e)) handleEvents(e, cam, input, quit, w, h);
        SDL_GetWindowSize(window, &w, &h);
        renderParametric(renderer, expressionX, expressionY, lineColour, w, h, cam);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}



void openMenu(Camera& cam) {
    const std::string logo1 = "\x1b[38;2;20;148;103m";
    const std::string logo2 = "\x1b[38;2;133;128;50m";
    const std::string col1 = "\x1b[38;2;131;165;152m";
    const std::string col2 = "\x1b[38;2;184;187;38m";
    const std::string col3 = "\x1b[38;2;211;134;155m";
    const std::string col4 = "\x1b[38;2;251;73;52m";
    const std::string prompt = "\x1b[38;2;250;189;47m";
    const std::string body = "\x1b[38;2;235;219;178m";
    const std::string reset = "\x1b[0m";

    std::stack<std::string> hist;
    std::string func1, func2, lineColour;

    printf(
        "%s ___ _       ___ _     _   _           %s \n"
        "%s/ __(_)_ __ | _ \\ |___| |_| |_ ___ _ _ %s\n"
        "%s\\__ \\ | '  \\|  _/ / _ \\  _|  _/ -_) '_|%s\n"
        "%s|___/_|_|_|_|_| |_\\___/\\__|\\__\\___|_|  %s\n\n",
        logo1.c_str(), reset.c_str(),
        logo2.c_str(), reset.c_str(),
        logo1.c_str(), reset.c_str(),
        logo2.c_str(), reset.c_str()
    );

    for (;;) {
        int choice = 0, param = 0;

        std::cout << body << "What do you want?" << reset << "\n"
                  << col1 << "1. Plotting (regular, parametric)" << reset << "\n"
                  << col2 << "2. Calculation" << reset << "\n"
                  << col3 << "3. List older expressions" << reset << "\n"
                  << col4 << "4. Exit" << reset << "\n"
                  << body << "Your Choice: " << prompt;

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        std::cout << reset << "\n";

        switch (choice) {
        case 1:
            std::cout << body << "Plotting" << reset << "\n"
                      << col1 << "1. Regular" << reset << "\n"
                      << col2 << "2. Parametric" << reset << "\n"
                      << body << "Your Choice: " << prompt;
            std::cin >> param;
            std::cout << reset << "\n";

            switch (param) {
            case 1:
                std::cout << body << "Regular function (ex. sin(x)/x^2 red)" << reset << "\n"
                          << col1 << "Enter:  <Function> <Line colour> " << prompt;
                std::cin >> func1 >> lineColour;
                std::cout << reset << "\n";
                hist.push(func1 + " " + lineColour);
                funcToGraph(func1, lineColour, cam);
                break;
            case 2:
                std::cout << body << "Parametric function (ex. sin(x) cos(x) magenta)" << reset << "\n"
                          << col1 << "Enter:  <x(t)> <y(t)> <Line colour> " << prompt;
                std::cin >> func1 >> func2 >> lineColour;
                std::cout << reset << "\n";
                hist.push(func1 + " " + func2 + " " + lineColour);
                funcToGraphParametric(func1, func2, lineColour, cam);
                break;
            default:
                std::cout << "\x1b[0;31mInvalid choice\x1b[0m\n";
                break;
            }
            break;

        case 2: {
            std::cout << body << "Enter an expression: " << prompt;
            std::cin.ignore();
            std::getline(std::cin, func1);
            std::cout << reset << "\n";
            float res;
            if (evaluateWithX(func1, 0.0f, res))
                std::cout << col2 << func1 << " = " << res << reset << "\n\n";
            else
                std::cout << "\x1b[0;31mInvalid expression\x1b[0m\n\n";
            hist.push(func1 + " = " + std::to_string(res));
            break;
        }

        case 3:
            if (hist.empty()) {
                std::cout << "No history yet.\n\n";
            } else {
                std::stack<std::string> tmp = hist;
                int k = 0;
                std::cout << body << "<< Operation history list (from newer to older) >>" << reset << "\n";
                while (!tmp.empty()) {
                    std::cout << col2 << ++k << ". " << tmp.top() << reset << "\n";
                    tmp.pop();
                }
                std::cout << "\n";
            }
            break;

        case 4:
            return;

        default:
            std::cout << "\x1b[0;31mInvalid choice\x1b[0m\n\n";
            break;
        }
    }
}


int main() {
    SDL_Init(SDL_INIT_VIDEO);
    Camera cam;
    openMenu(cam);
    printf("Bye...\n");
    SDL_Quit();
    return 0;
}