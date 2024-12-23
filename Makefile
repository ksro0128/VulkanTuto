NAME :=	VULKAN_TUTORIAL

all: $(NAME)_debug

release: $(NAME)_release

$(NAME)_debug:
	@cmake -Bbuild -DCMAKE_BUILD_TYPE=Debug .
	@cmake --build build --config Debug
	@mv ./build/Debug/$(NAME).exe ./$(NAME)_debug.exe
	@echo [SUCCESS] $@ compiled successfully with debug mode and validation layers!

$(NAME)_release:
	@cmake -Bbuild -DCMAKE_BUILD_TYPE=Release .
	@cmake --build build --config Release
	@mv ./build/Release/$(NAME).exe ./$(NAME)_release.exe
	@echo [SUCCESS] $@ compiled successfully without validation layers!

clean:
	@rm -rf ./build/
	@echo [CLEAN] Build files have been removed!

fclean: clean
	@rm -rf $(NAME)_debug.exe $(NAME)_release.exe
	@echo [FCLEAN] Executable files have been fully removed!

re: fclean all

shader :
# shaders 폴더안에 있는 모든 .vert .frag 파일을 glslc로 컴파일 해서 /spv 폴더에 넣는다
	@mkdir -p ./spvs
	@for file in ./shaders/*.vert; do \
		glslc $$file -o ./spvs/$$(basename $$file .vert).vert.spv; \
	done
	@for file in ./shaders/*.frag; do \
		glslc $$file -o ./spvs/$$(basename $$file .frag).frag.spv; \
	done
	@echo [SUCCESS] Shaders have been compiled successfully!


.PHONY: all clean fclean re debug release shader