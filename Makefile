CC = c++
NAME = webserv
FLAGS = -Wall  -Wextra -std=c++98 #-fsanitize=address

UTILS = Utils.cpp
SOCKET =  TcpServer.cpp
SERVER_SRC = $(addprefix utils/, $(UTILS)) $(addprefix socket/, $(SOCKET))
SRC = main.cpp  $(addprefix Server/, $(SERVER_SRC))
OBJ = $(SRC:.cpp=.o)


all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(CC) $(FLAGS) -c $< -o $@

re: fclean all

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)
