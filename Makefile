# General
NAME = webserv
COMPILER = c++
FLAGS = -Wall -Wextra -std=c++98 #-fsanitize=address -g

# FileUpload part
FILE_UPLOAD_SRC = FileUpload.cpp
FILE_UPLOAD_HEADERS = FileUpload.hpp

# Server part
SERVER_HEADERS = TcpServer.hpp
UTILS = Utils.cpp
SERVER_SRC = TcpServer.cpp

# Client Part
CLIENT_SRC = Client.cpp Config.cpp
CLIENT_HEADERS = Client.hpp Config.hpp

HEADERS = $(addprefix Client/, $(CLIENT_HEADERS)) $(addprefix FileUpload/, $(FILE_UPLOAD_HEADERS)) $(addprefix Server/, $(SERVER_HEADERS))
SRC = main.cpp  $(addprefix Server/, $(SERVER_SRC)) $(addprefix Client/, $(CLIENT_SRC)) $(addprefix FileUpload/, $(FILE_UPLOAD_SRC))
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
	rm -f $(NAME) *.txt
