# General
NAME = webserv
COMPILER = c++
FLAGS = -Wall -Wextra -std=c++98 #-fsanitize=address -g

# Server part
UTILS = Utils.cpp
SOCKET =  TcpServer.cpp
SERVER_SRC = $(addprefix utils/, $(UTILS)) $(addprefix socket/, $(SOCKET))

# Client Part
CLIENT_SRC = Client.cpp
CLIENT_HEADERS = Client.hpp

HEADERS = $(addprefix Client/, $(CLIENT_HEADERS))
SRC = main.cpp  $(addprefix Server/, $(SERVER_SRC)) $(addprefix Client/, $(CLIENT_SRC))
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
