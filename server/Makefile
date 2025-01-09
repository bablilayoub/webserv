CC = c++
NAME = server
FLAGS = -Wall  -Wextra -std=c++98 -fsanitize=address

UTILS = Utils.cpp
SOCKET =  TcpServer.cpp
SRC = server.cpp $(addprefix utils/, $(UTILS)) $(addprefix socket/, $(SOCKET))
OBJ = $(SRC:.cpp=.o)


all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)
