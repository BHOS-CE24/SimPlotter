#include <cmath>
#include <string>
#include <cstdint>
#include <SDL2/SDL.h>
#include <stack>
#include <iostream>
#include <algorithm>
#include <cstdint>
#include <ctime>

#define E 2.71828182845904523536
#define PI 3.14159265358979323846



namespace Operation {

float applyOp(float a, float b, char op) {
	switch (op) {
	case '+':
		return a + b;
	case '-':
		return a - b;
	case '*':
		return a * b;
	case '/':
		if (b == 0) {
			return 0;
		}
		return a / b;
	case '%':
		if (b == 0) {
			return 0;
		}
		return (int)a % (int)b;
	case '^':
		return pow(a, b);
	}
	return 0;
}

}

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
	if (func == "sqrt") return sqrt(value);
	if (func == "sin") return sin(value);
	if (func == "cos") return cos(value);
	if (func == "tan") return tan(value);
	if (func == "asin") return asin(value);
	if (func == "acos") return acos(value);
	if (func == "atan") return atan(value);
	if (func == "sinh") return sinh(value);
	if (func == "cosh") return cosh(value);
	if (func == "tanh") return tanh(value);
	if (func == "log") return log10(value);
	if (func == "ln") return log(value);
	if (func == "exp") return exp(value);
	if (func == "abs") return abs(value);
	if (func == "ceil") return ceil(value);
	if (func == "floor") return floor(value);
	if (func == "round") return round(value);
	return value;
}

int precedence(char op) {
	if (op == '+' || op == '-') return 1;
	if (op == '*' || op == '/' || op == '%') return 2;
	if (op == '^') return 3;
	return 0;
}


bool isRightAssociative(char op) {
	return (op == '^');
}

bool isFunction(const std::string& expr, int pos, const std::string& func) {
	int len = func.length();
	if (pos + len > (int)expr.length()) return false;
	std::string substr = expr.substr(pos, len);
	std::transform(substr.begin(), substr.end(), substr.begin(), ::tolower);
	return substr == func;
}

std::string getFunctionName(const std::string& expr, int pos) {
	std::string functions[] = {
		"sqrt", "asin", "acos", "atan", "sinh", "cosh", "tanh",
		"sin", "cos", "tan", "log", "ln", "exp",
		"abs", "ceil", "floor", "round"
	};
	for (int i = 0; i < 17; i++) {
		if (isFunction(expr, pos, functions[i])) {
			return functions[i];
		}
	}
	return "";
}

template<typename D>
D factorial(D a) {
	if( a<D(0)) return D(0);

	D f =D(1);
	int n = (int)round((float)a);

	if (n>20) return D((__builtin_inff()));

	for ( int i = 2; i<=n; ++i) {
		f *= D(i);
	}
	return f;
}

template<typename A, typename B>
bool applyTop (std::stack<A>& values, std::stack<B>& ops) {
	if (values.size() < 2 || ops.empty()) return false;
	float b = values.top();
	values.pop();
	float a = values.top();
	values.pop();
	char  op = ops.top();
	ops.pop();
	values.push(Operation::applyOp(a, b, op));
	return true;
}

bool evaluateWithX(const std::string& expression, float xValue, float& result) {
	std::string expr = "";
	for (int i = 0; i < (int)expression.length(); i++) {
		if (expression[i] == 'x') {
			bool isStandalone = true;
			if (i > 0 && (isalpha(expression[i-1]) || isdigit(expression[i-1]))) {
				isStandalone = false;
			}
			if (i < (int)expression.length() - 1 && (isalpha(expression[i+1]) || isdigit(expression[i+1]))) {
				isStandalone = false;
			}

			if (isStandalone) {
				char buf[32];
				sprintf(buf, "(%.6f)", xValue);
				expr += buf;
			} else {
				expr += "x";
			}
		} else {
			expr += expression[i];
		}
	}

	expr.erase(std::remove(expr.begin(), expr.end(), ' '), expr.end());

	size_t pos = expr.find("pi");
	while (pos != std::string::npos) {
		expr.replace(pos, 2, std::to_string(PI));
		pos = expr.find("pi", pos + std::to_string(PI).length());
	}
	pos = 0;
	while ((pos = expr.find('e', pos))!= std::string::npos) {
		bool standalone = true;
		if (pos > 0 && (isalpha(expr[pos-1]) || isdigit(expr[pos-1])))
			standalone = false;
		if (pos + 1 < expr.length() && (isalpha(expr[pos+1]) || isdigit(expr[pos+1])))
			standalone = false;
		if (standalone) {
			std::string eStr = std::to_string(E);
			expr.replace(pos, 1, eStr);
			pos += eStr.length();
		} else {
			pos++;
		}
	}
	if (expr.length() == 0) return false;

	std::stack<float>       values;
	std::stack<char>        ops;
	std::stack<std::string> funcs;



	int n = expr.length();
	for (int i = 0; i < n; i++) {

		std::string funcName = getFunctionName(expr, i);
		if (!funcName.empty()) {
			i += funcName.length();
			funcs.push(funcName);
			continue;
		}

		if (expr[i] == '(') {
			ops.push('(');
		}
		else if (expr[i] == ')') {
			while (!ops.empty() && ops.top() != '(') {
				if (!applyTop(values,ops)) return false;
			}
			if (!ops.empty()) ops.pop();

			if (!funcs.empty()) {
				if (values.empty()) return false;
				float val = values.top();
				values.pop();
				values.push(applyFunction(funcs.top(), val));
				funcs.pop();
			}
		}
		else if (isdigit(expr[i]) || expr[i] == '.') {
			float num = 0;
			float decimal = 0;
			bool hasDecimal = false;
			int decimalPlaces = 0;

			while (i < n && (isdigit(expr[i]) || expr[i] == '.')) {
				if (expr[i] == '.') {
					if (hasDecimal) return false;
					hasDecimal = true;
				} else {
					if (hasDecimal) {
						decimal = decimal * 10 + (expr[i] - '0');
						decimalPlaces++;
					} else {
						num = num * 10 + (expr[i] - '0');
					}
				}
				i++;
			}
			i--;

			if (hasDecimal && decimalPlaces > 0) {
				num += decimal / pow(10, decimalPlaces);
			}
			values.push(num);
		}
		else if (expr[i] == '-' && (i == 0 || expr[i-1] == '(' ||
		                            expr[i-1] == '+' || expr[i-1] == '-' ||
		                            expr[i-1] == '*' || expr[i-1] == '/' ||
		                            expr[i-1] == '^')) {
			values.push(0.0f);
			ops.push('-');
		}
		else if (expr[i] == '+' || expr[i] == '-' ||
		         expr[i] == '*' || expr[i] == '/' ||
		         expr[i] == '%' || expr[i] == '^') {
			while (!ops.empty() && ops.top() != '(' &&
			        (precedence(ops.top()) > precedence(expr[i]) ||
			         (precedence(ops.top()) == precedence(expr[i]) &&
			          !isRightAssociative(expr[i])))) {
				if (!applyTop(values,ops)) return false;
			}
			ops.push(expr[i]);
		} else if (expr[i] == '!')
		{
			if(values.empty()) return false;
			float val = values.top();
			values.pop();
			values.push(factorial(val));
		}

		else {
			return false;
		}
	}

	while (!ops.empty()) {
		if (ops.top() == '(') return false;
		if (!applyTop(values,ops)) return false;
	}

	if (values.size() != 1) return false;
	result = values.top();
	return true;
}

void renderAction(SDL_Renderer* renderer, const std::string& expression,
                  const std::string& lineColour, int w, int h, Camera& cam ) {


	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderClear(renderer);

	float pixelsPerUnit = (float)w / cam.range;
	int centerX = (int)roundf(w / 2.0f - cam.Xoffset * pixelsPerUnit);
	int centerY = (int)roundf(h / 2.0f + cam.Yoffset * pixelsPerUnit);

	float gridSpacing = 1.0f;

	float half = cam.range / 2.0f;
	const float xMin = cam.Xoffset - half;
	const float xMax = cam.Xoffset + half;
	const float yMin = cam.Yoffset - half;
	const float yMax = cam.Yoffset + half;




	for (float i = xMin; i <= xMax; i += gridSpacing) {
		int px = centerX + (int)(i * pixelsPerUnit);
		if (px >= 0 && px < w) {
			if ( fabsf(i) <= 0.001f) SDL_SetRenderDrawColor(renderer,190,190,190,255);
			else SDL_SetRenderDrawColor(renderer,200,200,200,250);

			SDL_RenderDrawLine(renderer, px, 0, px, h);
		}
	}
	for (float i = yMin; i <= yMax; i+= gridSpacing) {
		int py = centerY - (int)(i * pixelsPerUnit);
		if (py >= 0 && py < h) {
			if ( fabsf(i) <= 0.001f) SDL_SetRenderDrawColor(renderer,190,190,190,255);
			else SDL_SetRenderDrawColor(renderer,205,205,205,255);

			SDL_RenderDrawLine(renderer, 0, py, w, py);
		}
	}

	SDL_SetRenderDrawColor(renderer,0,0,0,255);

	SDL_RenderDrawLine(renderer, 0, centerY, w, centerY);
	SDL_RenderDrawLine(renderer, 0,  centerY + 1, w, centerY + 1);
	SDL_RenderDrawLine(renderer, centerX, 0, centerX, h);
	SDL_RenderDrawLine(renderer, centerX + 1, 0, centerX + 1, h);



	int r, g, b;
	if (lineColour == "red") {
		r=255;
		g=0;
		b=0;
	}
	else if (lineColour == "green") {
		r=0;
		g=255;
		b=0;
	}
	else if (lineColour == "blue") {
		r=0;
		g=0;
		b=255;
	}
	else if (lineColour == "black") {
		r=0;
		g=0;
		b=0;
	}
	else if (lineColour == "yellow") {
		r=255;
		g=255;
		b=0;
	}
	else if (lineColour == "cyan") {
		r=0;
		g=255;
		b=255;
	}
	else if (lineColour == "magenta") {
		r=255;
		g=0;
		b=255;
	}
	else if (lineColour == "orange") {
		r=255;
		g=160;
		b=0;
	}
	else if (lineColour == "purple") {
		r=225;
		g=0;
		b=255;
	}
	else {
		r=0;
		g=0;
		b=255;
	}
	SDL_SetRenderDrawColor(renderer, r, g, b,255);
	float prevY = NAN;
	int prevPx = -1;
	int prevPy = -1;

	for (int px = 0; px < w; px++) {
		float mathX = (px - centerX) / pixelsPerUnit;

		float mathY;
		if (evaluateWithX(expression, mathX, mathY)) {
			if (isnan(mathY) || isinf(mathY)) {
				prevY = NAN;
				continue;
			}

			int py = centerY - (int)(mathY * pixelsPerUnit);

			if (py >= 0 && py < h) {
				if (!isnan(prevY) && prevPx >= 0) {
					if (abs(py - prevPy) < h / 2) {

						SDL_RenderDrawLine(renderer,prevPx, prevPy, px, py);
					} else {
						SDL_RenderDrawPoint(renderer,px, py);
					}
				} else {
					SDL_RenderDrawPoint(renderer,px, py);
				}

				prevY = mathY;
				prevPx = px;
				prevPy = py;
			} else {
				prevY = NAN;
			}
		} else {
			prevY = NAN;
		}
	}

	SDL_RenderPresent(renderer);
}


void renderParametric(SDL_Renderer* renderer, const std::string& exprX,
                      const std::string& exprY, const std::string& lineColour, int w, int h,Camera& cam) {

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderClear(renderer);

	float pixelsPerUnit = (float)w / cam.range;
	int centerX = (int)roundf(w / 2.0f - cam.Xoffset * pixelsPerUnit);
	int centerY = (int)roundf(h / 2.0f + cam.Yoffset * pixelsPerUnit);

	float gridSpacing = 1.0f;

	float half = cam.range / 2.0f;
	const float xMin = cam.Xoffset - half;
	const float xMax = cam.Xoffset + half;
	const float yMin = cam.Yoffset - half;
	const float yMax = cam.Yoffset + half;

	for (float i = xMin; i <= xMax; i += gridSpacing) {
		int px = centerX + (int)(i * pixelsPerUnit);
		if (px >= 0 && px < w) {
			if (fabsf(i) <= 0.001f) SDL_SetRenderDrawColor(renderer, 190, 190, 190, 255);
			else                    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
			SDL_RenderDrawLine(renderer, px, 0, px, h);
		}
	}
	for (float i = xMin; i <= xMax; i += gridSpacing) {
		int py = centerY - (int)(i * pixelsPerUnit);
		if (py >= 0 && py < h) {
			if (fabsf(i) <= 0.001f) SDL_SetRenderDrawColor(renderer, 190, 190, 190, 255);
			else                    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
			SDL_RenderDrawLine(renderer, 0, py, w, py);
		}
	}

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderDrawLine(renderer, 0, centerY,     w, centerY);
	SDL_RenderDrawLine(renderer, 0, centerY + 1, w, centerY + 1);
	SDL_RenderDrawLine(renderer, centerX,     0, centerX,     h);
	SDL_RenderDrawLine(renderer, centerX + 1, 0, centerX + 1, h);

	int r, g, b;
	if (lineColour == "red") {
		r=255;
		g=0;
		b=0;
	}
	else if (lineColour == "green") {
		r=0;
		g=255;
		b=0;
	}
	else if (lineColour == "blue") {
		r=0;
		g=0;
		b=255;
	}
	else if (lineColour == "black") {
		r=0;
		g=0;
		b=0;
	}
	else if (lineColour == "yellow") {
		r=255;
		g=255;
		b=0;
	}
	else if (lineColour == "cyan") {
		r=0;
		g=255;
		b=255;
	}
	else if (lineColour == "magenta") {
		r=255;
		g=0;
		b=255;
	}
	else if (lineColour == "orange") {
		r=255;
		g=160;
		b=0;
	}
	else if (lineColour == "purple") {
		r=128;
		g=0;
		b=255;
	}
	else {
		r=0;
		g=0;
		b=255;
	}
	SDL_SetRenderDrawColor(renderer, r, g, b, 255);

	float tMin = 0.0f;
	float tMax = 2.0f * PI;
	int steps = 10000;
	float dt = (tMax - tMin) / steps;

	int prevPx = -1;
	int prevPy = -1;
	bool hasPrev = false;

	for (int i = 0; i <= steps; i++) {
		float t = tMin + i * dt;
		float mathX, mathY;

		if (evaluateWithX(exprX, t, mathX) && evaluateWithX(exprY, t, mathY)) {
			if (isnan(mathX) || isinf(mathX) || isnan(mathY) || isinf(mathY)) {
				hasPrev = false;
				continue;
			}

			int px = centerX + (int)(mathX * pixelsPerUnit);
			int py = centerY - (int)(mathY * pixelsPerUnit);

			if (px >= 0 && px < w && py >= 0 && py < h) {
				if (hasPrev) {
					SDL_RenderDrawLine(renderer, prevPx, prevPy, px, py);
				} else {
					SDL_RenderDrawPoint(renderer, px, py);
				}
				prevPx = px;
				prevPy = py;
				hasPrev = true;
			} else {
				hasPrev = false;
			}
		} else {
			hasPrev = false;
		}
	}

	SDL_RenderPresent(renderer);
}


void funcToGraph(const std::string& expression, const std::string& lineColour, Camera& cam) {
	SDL_Window* window = SDL_CreateWindow("Simple Plotter",
	                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280,
	                                      720, SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_Event e;
	bool quit = false;
	State input;
	int w = 1280, h = 720;



	while (!quit) {
		SDL_GetWindowSize(window, &w, &h);
		float pixel = float(w)/cam.range;
		float step = cam.range / 20.f;

		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) quit = true;
			if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) quit = true;
			if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym)
				{

				case SDLK_LEFT:
					cam.Xoffset -= step;
					break;
				case SDLK_RIGHT:
					cam.Xoffset += step;
					break;
				case SDLK_UP:
					cam.Yoffset += step;
					break;
				case SDLK_DOWN:
					cam.Yoffset -= step;
					break;

				}
			}
			if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_MIDDLE) {
				cam.Xoffset = 0;
				cam.Yoffset = 0;
				cam.range = 50.0f;
			}

			if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_HOME) {
				cam.Xoffset = 0;
				cam.Yoffset = 0;
				cam.range = 50.0f;
			}
			if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
				input.moved   = true;
				input.dragStartX = e.button.x;
				input.dragStartY = e.button.y;
				input.camPositionX = cam.Xoffset;
				input.camPositionY = cam.Yoffset;
			}
			if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
				input.moved = false;
			}
			if (e.type == SDL_MOUSEMOTION && input.moved) {
				int dx = e.motion.x - input.dragStartX;
				int dy = e.motion.y - input.dragStartY;
				cam.Xoffset = input.camPositionX - dx / pixel;
				cam.Yoffset = input.camPositionY + dy / pixel;
			}

			if (e.type == SDL_MOUSEWHEEL) {
				int mouseX, mouseY;
				SDL_GetMouseState(&mouseX, &mouseY);
				int originX = (int)(w / 2.0f - cam.Xoffset * pixel);
				int originY = (int)(h / 2.0f + cam.Yoffset * pixel);
				float mathX = (mouseX - originX) / pixel;
				float mathY = (originY - mouseY) / pixel;
				float zoomFactor = (e.wheel.y > 0) ? 0.9f : 1.1f;
				cam.range   *= zoomFactor;
				cam.Xoffset += mathX * (1.0f - zoomFactor);
				cam.Yoffset += mathY * (1.0f - zoomFactor);
			}
		}

		SDL_GetWindowSize(window,&w,&h);

		renderAction(renderer, expression, lineColour, w, h,cam);
		SDL_Delay(10);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}

void funcToGraphParametric(const std::string& expressionX,const std::string& expressionY, const std::string& lineColour, Camera& cam) {
	SDL_Window* window = SDL_CreateWindow("Simple Plotter",
	                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280,
	                                      720, SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_Event e;
	bool quit = false;
	State input;
	int w = 1280, h=720;

	while (!quit) {
		SDL_GetWindowSize(window, &w, &h);
		float pixel = float(w)/cam.range;
		float step = cam.range / 20.f;
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) quit = true;
			if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) quit = true;
			if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym)
				{

				case SDLK_LEFT:
					cam.Xoffset -= step;
					break;
				case SDLK_RIGHT:
					cam.Xoffset += step;
					break;
				case SDLK_UP:
					cam.Yoffset += step;
					break;
				case SDLK_DOWN:
					cam.Yoffset -= step;
					break;

				}
			}
			if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_MIDDLE) {
				cam.Xoffset = 0;
				cam.Yoffset = 0;
				cam.range = 50.0f;
			}

			if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_HOME) {
				cam.Xoffset = 0;
				cam.Yoffset = 0;
				cam.range = 50.0f;
			}
			if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
				input.moved   = true;
				input.dragStartX = e.button.x;
				input.dragStartY = e.button.y;
				input.camPositionX = cam.Xoffset;
				input.camPositionY = cam.Yoffset;
			}
			if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
				input.moved = false;
			}
			if (e.type == SDL_MOUSEMOTION && input.moved) {
				int dx = e.motion.x - input.dragStartX;
				int dy = e.motion.y - input.dragStartY;
				cam.Xoffset = input.camPositionX - dx / pixel;
				cam.Yoffset = input.camPositionY + dy / pixel;
			}

			if (e.type == SDL_MOUSEWHEEL) {
				int mouseX, mouseY;
				SDL_GetMouseState(&mouseX, &mouseY);
				int originX = (int)(w / 2.0f - cam.Xoffset * pixel);
				int originY = (int)(h / 2.0f + cam.Yoffset * pixel);
				float mathX = (mouseX - originX) / pixel;
				float mathY = (originY - mouseY) / pixel;
				float zoomFactor = (e.wheel.y > 0) ? 0.9f : 1.1f;
				cam.range   *= zoomFactor;
				cam.Xoffset += mathX * (1.0f - zoomFactor);
				cam.Yoffset += mathY * (1.0f - zoomFactor);
			}
		}
		SDL_GetWindowSize(window, &w, &h);
		renderParametric(renderer, expressionX, expressionY, lineColour, w, h,cam);
		SDL_Delay(16);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
}


void openMenu(Camera& cam) {


    std::string logo1 = "\x1b[38;2;20;148;103m";   
    std::string logo2 = "\x1b[38;2;133;128;50m";   

    std::string col1 = "\x1b[38;2;131;165;152m";  
    std::string col2 = "\x1b[38;2;184;187;38m";   
    std::string col3 = "\x1b[38;2;211;134;155m";  
    std::string col4 = "\x1b[38;2;251;73;52m";    

    std::string bg = "\x1b[48;2;60;56;54m";     

    std::string prompt = "\x1b[38;2;250;189;47m";   
    std::string body = "\x1b[38;2;235;219;178m";  


	std::string reset = "\x1b[0m";

	std::stack<std::string> hist = {};

	std::string func1;
	std::string func2;
	std::string lineColour;


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

	for(;;) {

		int choice, param;

		std::cout << body
		          << "What do you want?"
                  << reset 
                  << "\n"
		          << col1
		          << "1. Plotting (regular, parametric)"
		          << reset
		          << "\n"
		          << col2
		          << "2. Calculation"
		          << reset
		          << "\n"
		          << col3
		          << "3. List older expressions"
		          << reset
		          << "\n"
		          << col4
		          << "4. Exit"
		          << reset
		          << "\n"
		          << body
		          << "Your Choice: "
		          << prompt;
		if (!(std::cin >> choice)) {
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			continue;
		}
		std::cout << reset << "\n";



		switch (choice)
		{
		case 1:
			std::cout << body << "Plotting" << reset << "\n" << col1
			          << "1. Regular" << reset << "\n" << col2
			          << "2. Parametric" << reset << "\n"
			          << reset << "\n"
			          << body << "Your Choice: " << prompt;
			std::cin >> param;
            std::cout << reset;
			switch (param)
			{
			case 1:
				std::cout << body << "Regular function (ex. (sin(x)/x^2 red) )" << reset << "\n" << col1
				          << "Enter:  <Function> <Line colour> " << prompt;
				std::cin >> func1 >> lineColour;
				hist.push(func1+" "+lineColour);
				funcToGraph(func1, lineColour, cam);
                std::cout << reset;
				break;
			case 2:
				std::cout << body << "Parametric function (ex. (sin(x)/x^2 e^x magenta) )" << reset << "\n" << col1
				          << "Enter:  <Function y(t)> <Function y(t)> <Line colour> " << prompt;
				std::cin >> func1 >> func2 >> lineColour;
                std::cout << reset;
				hist.push(func1+" "+func2 +" "+lineColour);
				funcToGraphParametric(func1, func2, lineColour,cam);

				break;
			default:
				break;
			}

			break;
		case 2:
			std::cout << body << "Enter an expression: " << prompt;
			std::cin.ignore();
			std::getline(std::cin, func1);
            std::cout << reset;
			float result;

			if (evaluateWithX(func1, 0.0f, result)) {
				std::cout << col2 << func1 << " = " << result << reset << "\n";
				hist.push(func1 + " = " + std::to_string(result));
			}

			else std::cout << "\x1b[0;31m" <<"Invalid expression" << reset << "\n";
			break;


		case 3:

			if (hist.empty()) {
				std::cout << "No history yet.\n";
			}
			else {
				std::stack<std::string> tmp = hist;
				int k = 0;
				std::cout << body <<"<< Operation history list (from newer to older) >>" <<reset << "\n";
				while (!tmp.empty()) {
					std::cout << col2
					          << k+1
					          << ". "
					          << tmp.top()
					          << reset
					          << "\n" ;
					tmp.pop();
				}
				std::cout << "\n" ;
			}
			break;

		case 4:
			return;

		default:
			std::cout << "\x1b[0;31m" << "Invalid choice" << reset << "\n";
			break;
		}
	}
}


int main() {
	SDL_Init(SDL_INIT_VIDEO);
	Camera cam;

	openMenu(cam);
	printf("Bye...\n");

	// printf("Usage:\n");
	// printf("  plotter \"expression\" \"colour\"\n");
	// printf("  plotter -p \"x(t)\" \"y(t)\" \"colour\"\n");

	SDL_Quit();
	return 0;
}