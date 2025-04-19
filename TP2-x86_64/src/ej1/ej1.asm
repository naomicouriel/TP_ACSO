section .text
global string_proc_list_create_asm
global string_proc_node_create_asm
global string_proc_list_add_node_asm
global string_proc_list_concat_asm

extern malloc
extern free
extern strdup
extern str_concat

; string_proc_list* string_proc_list_create_asm(void)
string_proc_list_create_asm:
    push rbp
    mov rbp, rsp
    
    mov rdi, 16           ; sizeof(string_proc_list) = 16 (2 punteros)
    call malloc
    
    test rax, rax
    jz .end
    
    ; Inicializar lista vacía
    mov qword [rax], 0     ; first = NULL (offset 0)
    mov qword [rax+8], 0   ; last = NULL (offset 8)
    
.end:
    pop rbp
    ret

; string_proc_node* string_proc_node_create_asm(uint8_t type, char* hash)
string_proc_node_create_asm:
    push rbp
    mov rbp, rsp
    push r12
    push r13
    
    ; Guardar parámetros
    mov r12b, dil          ; type (uint8_t)
    mov r13, rsi           ; hash (char*)
    
    ; Reservar memoria para el nodo
    mov rdi, 32            ; sizeof(string_proc_node) = 32 bytes
    call malloc
    
    test rax, rax
    jz .error
    
    ; Inicializar nodo
    mov qword [rax], 0      ; next = NULL (offset 0)
    mov qword [rax+8], 0    ; previous = NULL (offset 8)
    mov byte [rax+16], r12b ; type (offset 16)
    mov qword [rax+24], r13 ; hash (offset 24)
    
.error:
    pop r13
    pop r12
    pop rbp
    ret

; void string_proc_list_add_node_asm(string_proc_list* list, uint8_t type, char* hash)
string_proc_list_add_node_asm:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    
    mov rbx, rdi          ; list
    mov r12b, sil         ; type
    mov r13, rdx          ; hash
    
    ; Crear nuevo nodo
    movzx rdi, r12b       ; type
    mov rsi, r13          ; hash
    call string_proc_node_create_asm
    
    test rax, rax
    jz .end
    
    mov r14, rax          ; nuevo nodo
    
    ; Verificar si la lista está vacía
    cmp qword [rbx], 0    ; list->first == NULL?
    jne .not_empty
    
    ; Lista vacía
    mov [rbx], r14        ; list->first = new_node
    mov [rbx+8], r14      ; list->last = new_node
    jmp .end
    
.not_empty:
    ; Insertar al final
    mov rcx, [rbx+8]      ; list->last
    mov [r14+8], rcx      ; new_node->previous = last
    mov [rcx], r14        ; last->next = new_node
    mov [rbx+8], r14      ; list->last = new_node
    
.end:
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

; char* string_proc_list_concat_asm(string_proc_list* list, uint8_t type, char* hash)
string_proc_list_concat_asm:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    push r15
    
    mov rbx, rdi          ; list
    mov r12b, sil         ; type
    mov r13, rdx          ; hash inicial
    
    ; strdup(hash)
    mov rdi, r13
    call strdup
    mov r14, rax          ; resultado
    test r14, r14
    jz .end
    
    ; Recorrer lista
    mov r15, [rbx]        ; current = list->first
    
.loop:
    test r15, r15
    jz .end
    
    ; Verificar tipo
    movzx eax, byte [r15+16] ; current->type
    cmp al, r12b
    jne .next
    
    ; Concatenar
    mov rdi, r14          ; resultado actual
    mov rsi, [r15+24]     ; current->hash
    call str_concat
    test rax, rax
    jz .concat_error
    
    ; Liberar string anterior
    mov rdi, r14
    mov r14, rax          ; nuevo resultado
    call free
    
.next:
    mov r15, [r15]        ; current = current->next
    jmp .loop
    
.concat_error:
    ; Liberar memoria en caso de error
    mov rdi, r14
    call free
    xor r14, r14          ; return NULL
    
.end:
    mov rax, r14
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret