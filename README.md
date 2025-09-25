# Trabalho Prático – Algoritmos e Programação II  
## Sistema de Gestão de Clínica Veterinária

---

## 1) Integrantes
- **Humberto Pimenta Ferreira Filho** - 12421BSI377  
- **Isaque Araujo Goncalves** – 12421BSI408  
- **Arthur Flávio Gonçalves Araújo** - 12421BSI378  
- **Joaquim Pedro Ferreira Cardoso** - 12421BSI382  
- **Lucas Gabriel dos Reis França** - 12421BSI397  

---

## 2) Objetivo do Sistema
Este projeto implementa, em linguagem **C**, um sistema de gerenciamento para uma clínica veterinária. O sistema foi desenvolvido de acordo com os requisitos fornecidos no enunciado do trabalho.

- São utilizadas três estruturas principais:
  - `ANIMAL`
  - `VETERINÁRIO`
  - `CONSULTA` (relaciona Animal e Veterinário)

- Cada estrutura possui operações de **CRUD** (Cadastrar, Consultar, Atualizar, Remover).
- A estrutura de **CONSULTA** possui filtros (por data, por CRM e por espécie do animal) e geração de relatórios em arquivos `.txt`.
- Os dados são armazenados em **vetores dinâmicos** (com realocação em blocos de 5 elementos) e são persistidos em **arquivos binários**.

---

## 3) Requisitos Atendidos
- CRUD completo das três estruturas.  
- Relacionamento entre elas (Consulta referencia Animal e Veterinário).  
- Relatórios em formato texto (`.txt`).  
- Persistência em arquivos binários, carregados no início e salvos no fim.  
- Arquivos binários pré-preenchidos com pelo menos **10 registros** de cada estrutura.  
- Verificações de integridade referencial (não é possível excluir Animal/Vet que possuam Consultas vinculadas).  

---

## 4) Compilação e Execução
**Ambiente utilizado:** CodeBlocks 25.03 (Windows), linguagem C padrão.  

**Passos:**
1. Abrir o projeto no CodeBlocks.  
2. Compilar.  
3. Executar o programa.  

---

## 5) Arquivos Utilizados
Durante a execução, o sistema utiliza três arquivos binários:

- `animais.bin`  
- `veterinarios.bin`  
- `consultas.bin`  

Esses arquivos são carregados automaticamente no início do programa e salvos no momento de sair (ou pela opção **6** do menu principal).

⚠️ **IMPORTANTE**:
- Se os arquivos não existirem, o programa inicia vazio.  
- Para gerar os arquivos já preenchidos com 10 registros em cada estrutura, utilize a opção **5 – Popular exemplos** no menu principal.  
- Depois de popular, escolha a opção de **sair** para salvar. Assim os arquivos `.bin` ficam gravados na pasta do executável.  
- **Observação**: Caso os 10 dados iniciais não sejam exibidos ao listar, verifique se os arquivos `veterinarios.bin`, `consultas.bin` e `animais.bin` estão na pasta **output**.  
  Esse problema pode ocorrer principalmente no **VSCode**, portanto é importante conferir.  

---

## 6) Estrutura dos Menus
**Menu Principal:**
1. Animais  
2. Veterinários  
3. Consultas  
4. Relatórios (TXT)  
5. Popular exemplos (gera 10 registros por estrutura)  
6. Salvar agora  
0. Sair (salva e encerra)  

Cada submenu oferece as operações de CRUD e consultas específicas.  

---

## 7) Observações
- IDs não são reordenados após exclusões.  
- O programa foi testado com operações de cadastro, atualização, remoção e geração de relatórios.  
- Relatórios `.txt` são gravados na mesma pasta do executável.