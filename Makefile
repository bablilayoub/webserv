# General
NAME = webserv
COMPILER = c++
FLAGS = -Wall -Wextra -std=c++98 #-fsanitize=address

# Server part
UTILS = Utils.cpp
SOCKET =  TcpServer.cpp
SERVER_SRC = $(addprefix utils/, $(UTILS)) $(addprefix socket/, $(SOCKET))

# Request Part
REQUEST_SRC = Request.cpp
REQUEST_HEADERS = Request.hpp

HEADERS = $(addprefix Request/, $(REQUEST_HEADERS))
SRC = main.cpp  $(addprefix Server/, $(SERVER_SRC)) $(addprefix Request/, $(REQUEST_SRC))
OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(COMPILER) $(FLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp $(HEADERS)
	$(COMPILER) $(FLAGS) -c $< -o $@

re: fclean all

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)
