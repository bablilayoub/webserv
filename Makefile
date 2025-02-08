# General
NAME = webserv
COMPILER = c++
FLAGS = -Wall -Wextra -Werror -std=c++98

# FileUpload part
FILE_UPLOAD_SRC = FileUpload.cpp
FILE_UPLOAD_HEADERS = FileUpload.hpp

# Server part
SERVER_HEADERS = WebServ.hpp
SERVER_SRC = WebServ.cpp

# Client Part
CLIENT_SRC = Client.cpp Config.cpp
CLIENT_HEADERS = Client.hpp Config.hpp Global.hpp

HEADERS = $(addprefix Client/, $(CLIENT_HEADERS)) $(addprefix FileUpload/, $(FILE_UPLOAD_HEADERS)) $(addprefix Server/, $(SERVER_HEADERS))
SRC = main.cpp  $(addprefix Server/, $(SERVER_SRC)) $(addprefix Client/, $(CLIENT_SRC)) $(addprefix FileUpload/, $(FILE_UPLOAD_SRC))
OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(COMPILER) $(FLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp $(HEADERS)
	$(COMPILER) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME) 

re: fclean all