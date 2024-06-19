#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#define MAX_CHILD 4 //4 way b+ tree
#define MAX_NODE 3

typedef struct Node
{

     int numKeys;            // Number of keys currently stored in the node
    int *keys;              // Array to store the keys
    char **names;           // Array to store the names
    char **departments;     // Array to store the departments
    union
    {
        struct Node **childNodes;   // Pointer to an array of child nodes
        FILE **dataPtr;             // Pointer to an array of data pointers (for leaf nodes)
    } ptrs;
    struct Node *parent;    // Pointer to the parent node
    int isLeaf;
} Node;

typedef struct BPTree
{
    int maxChildInt;        // Maximum number of child nodes for internal nodes
    int maxNodeLeaf;        // Maximum number of keys for leaf nodes
    Node *root;             // Pointer to the root node of the B+ tree
} BPTree;

typedef struct QueueNode
{
    Node *item;                 // Pointer to the node stored in the queue node
    struct QueueNode *next;     // Pointer to the next queue node
} QueueNode;

typedef struct Queue
{
    QueueNode *front;   // Pointer to the front (head) of the queue
    QueueNode *rear;    // Pointer to the rear (tail) of the queue
} Queue;



BPTree *createBPTree();
Node *createNode(int isLeaf);

// insert functions
void insert(BPTree *tree, int key, char *name, char *dept, FILE *filePtr);
void insertInternal(BPTree *tree, Node *parent, int key, char *name, char *dept, Node *right);
//void search(BPTree *bPTree, int key);
void search(BPTree *bPTree);

// remove functions
void removeKey(Node *root, int x);
Node *findParent(Node *root, Node *cursor);

// display functions
void display(Node *cursor,int blanks);

void insertAtBeginning(Node *node, int key, char *name, char *dept);
void insertAtEnd(Node *node, int key, char *name, char *dept);




int main()
{
    // Initialize the B+ tree
    BPTree *bPTree = createBPTree();

    printf("\n\n**************************************************************************************\n\n");
    printf("\t\tStudent Record Management using B+ Tree ");
    printf("\n\n**************************************************************************************\n\n");
    int option;
    do
    {
        printf("\nPlease provide the queries:\n\n");
        printf("Press 1 for : Insertion\n");
        printf("Press 2 for : Search\n");
        printf("Press 3 for : Print Tree\n");
        printf("Press 4 for : Delete Key In Tree\n");
        printf("Press 5 for : taking input from given txt file\n");
        printf("Press 6 for : EXIT.!!\n");
        scanf("%d", &option);

        switch (option)
        {
        case 1:
        {
            int rollNo;
            int age, marks;
            char name[100], dpt[100];

            printf("Please provide the rollNo: ");
            scanf("%d", &rollNo);

            printf("\nWhat's the Name : ");
            scanf("%s", name);
            printf("\nWhat's the Department : ");
            scanf("%s", dpt);
            printf("\nWhat's the Age : ");
            scanf("%d", &age);
            printf("\nWhat are the obtained marks : ");
            scanf("%d", &marks);

            char fileName[100];
            sprintf(fileName, "DBFiles/%d.txt", rollNo);
            FILE *filePtr = fopen(fileName, "w");
            fprintf(filePtr, "%s %s %d %d\n", name, dpt, age, marks);
            fclose(filePtr);

            insert(bPTree, rollNo, name, dpt, filePtr);

            printf("Insertion of roll No: %d Successful\n", rollNo);
            break;
        }
        case 2:
        {
            search(bPTree);
            break;
        }
        case 3:
        {
            printf("\nHere is your File Structure:\n");
            if (!bPTree->root)
            {
                printf("Tree is empty!\n");
            }
            else
            {
                display(bPTree->root, 0);
            }
            printf("\n");
            break;
        }
        case 4:
        {
            printf("Showing you the Tree, Choose a key from here:\n");
            display(bPTree->root, 0);

            int tmp;
            printf("Enter a key to delete: ");
            scanf("%d", &tmp);

            removeKey(bPTree->root, tmp);

            // Displaying
            // display(bPTree->root, 0);
            break;
        }
        case 5:
        {
            printf("Taking input from given input.txt file\n");
            // taking input from txt file
            FILE *inputFile = fopen("input.txt", "r");
            if (inputFile == NULL)
            {
                printf("Failed to open input file.\n");
                return 1;
            }

            char line[100];
            while (fgets(line, sizeof(line), inputFile) != NULL)
            {
                int rollNo, age, marks;
                char name[100], dpt[100];

                // Parse the line to extract the data
                sscanf(line, "%d,%[^,],%[^,],%d,%d", &rollNo, name, dpt, &age, &marks);

                // Create a file name based on the rollNo
                char fileName[100];
                sprintf(fileName, "DBFiles/%d.txt", rollNo);

                // Open the file for writing
                FILE *filePtr = fopen(fileName, "w");
                if (filePtr == NULL)
                {
                    printf("Failed to open file for rollNo %d.\n", rollNo);
                    continue;
                }

                // Write the data to the file
                fprintf(filePtr, "%s %s %d %d\n", name, dpt, age, marks);

                // Insert the key into the B+ tree
                insert(bPTree, rollNo, name, dpt, filePtr);

                // Close the file
                fclose(filePtr);
            }

            // Close the input file
            fclose(inputFile);
            printf("input.txt file is successfully read\n");
            break;
        }
        default:
            option = 6;
            break;
        }
    } while (option != 6);

    return 0;
}


BPTree *createBPTree()
{
    BPTree *bPTree = (BPTree *)malloc(sizeof(BPTree));
    bPTree->maxChildInt = MAX_CHILD;
    bPTree->maxNodeLeaf = MAX_NODE;
    bPTree->root = NULL;
    return bPTree;
}
Node *createNode(int isLeaf)
{

    Node *node = (Node *)malloc(sizeof(Node));

    node->numKeys = 0;

    node->keys = (int *)malloc((MAX_CHILD - 1) * sizeof(int));
    node->names = (char **)malloc((MAX_CHILD - 1) * sizeof(char *));
    node->departments = (char **)malloc((MAX_CHILD - 1) * sizeof(char *));

    if (isLeaf)
    {
        node->ptrs.dataPtr = (FILE **)malloc(MAX_NODE * sizeof(FILE *));
        // Allocate memory for the array of data pointers in the leaf node
    }
    else
    {
        node->ptrs.childNodes = (Node **)malloc(MAX_CHILD * sizeof(Node *));
        // Allocate memory for the array of child nodes in the internal node
    }

    node->parent = NULL;

    node->isLeaf = isLeaf;

    return node;
}







//--------------------------------------------- insert operations---------------------------------------------



void insert(BPTree *tree, int key, char *name, char *dept, FILE *filePtr)
{
    if (tree->root == NULL)
    {
        tree->root = createNode(1);
        tree->root->keys[0] = key;
        tree->root->names[0] = strdup(name);
        tree->root->departments[0] = strdup(dept);
        tree->root->ptrs.dataPtr[0] = filePtr;
        tree->root->numKeys = 1;

        printf("%d: I AM ROOT!!\n", key);
        return;
    }
    else
    {
        Node *cursor = tree->root;
        Node *parent = NULL;
        while (cursor->isLeaf == 0)
        {
            parent = cursor;
            for (int i = 0; i < cursor->numKeys; i++)
            {
                if (key < cursor->keys[i])
                {
                    cursor = cursor->ptrs.childNodes[i];
                    break;
                }
                if (i == cursor->numKeys - 1)
                {
                    cursor = cursor->ptrs.childNodes[i + 1];
                    break;
                }
            }
        }

        // Insert the key in the leaf node
        if (cursor->numKeys < MAX_NODE)
        {
            // Insert the key, name, and department in the correct position while maintaining sorted order
            int i = cursor->numKeys - 1;
            while (i >= 0 && key < cursor->keys[i])
            {
                cursor->keys[i + 1] = cursor->keys[i];
                cursor->names[i + 1] = cursor->names[i];
                cursor->departments[i + 1] = cursor->departments[i];
                cursor->ptrs.dataPtr[i + 1] = cursor->ptrs.dataPtr[i];
                i--;
            }
            cursor->keys[i + 1] = key;
            cursor->names[i + 1] = strdup(name);
            cursor->departments[i + 1] = strdup(dept);
            cursor->ptrs.dataPtr[i + 1] = filePtr;
            cursor->numKeys++;

            printf("%d: INSERTED INTO LEAF NODE\n", key);
        }
        else
        {
            // Leaf node is full, split it into two nodes
            Node *newLeaf = createNode(1);
            int tempKeys[MAX_NODE + 1];
            char *tempNames[MAX_NODE + 1];
            char *tempDepts[MAX_NODE + 1];
            FILE *tempPtrs[MAX_NODE + 1];
            for (int i = 0; i < MAX_NODE; i++)
            {
                tempKeys[i] = cursor->keys[i];
                tempNames[i] = strdup(cursor->names[i]);
                tempDepts[i] = strdup(cursor->departments[i]);
                tempPtrs[i] = cursor->ptrs.dataPtr[i];
            }
            int i = MAX_NODE - 1;
            while (i >= 0 && key < tempKeys[i])
            {
                tempKeys[i + 1] = tempKeys[i];
                tempNames[i + 1] = tempNames[i];
                tempDepts[i + 1] = tempDepts[i];
                tempPtrs[i + 1] = tempPtrs[i];
                i--;
            }
            tempKeys[i + 1] = key;
            tempNames[i + 1] = strdup(name);
            tempDepts[i + 1] = strdup(dept);
            tempPtrs[i + 1] = filePtr;

            cursor->numKeys = (MAX_NODE + 1) / 2;
            newLeaf->numKeys = MAX_NODE + 1 - (MAX_NODE + 1) / 2;

            for (int j = 0; j < cursor->numKeys; j++)
            {
                cursor->keys[j] = tempKeys[j];
                cursor->names[j] = tempNames[j];
                cursor->departments[j] = tempDepts[j];
                cursor->ptrs.dataPtr[j] = tempPtrs[j];
            }
            for (int j = 0, k = cursor->numKeys; j < newLeaf->numKeys; j++, k++)
            {
                newLeaf->keys[j] = tempKeys[k];
                newLeaf->names[j] = tempNames[k];
                newLeaf->departments[j] = tempDepts[k];
                newLeaf->ptrs.dataPtr[j] = tempPtrs[k];
            }

            newLeaf->ptrs.dataPtr[newLeaf->numKeys] = cursor->ptrs.dataPtr[MAX_NODE];
            cursor->ptrs.dataPtr[MAX_NODE] = (FILE *)newLeaf;

            if (cursor == tree->root)
            {
                // If the split occurs at the root, create a new root
                Node *newRoot = createNode(0);
                newRoot->keys[0] = newLeaf->keys[0];
                newRoot->names[0] = strdup(newLeaf->names[0]);
                newRoot->departments[0] = strdup(newLeaf->departments[0]);
                newRoot->ptrs.childNodes[0] = cursor;
                newRoot->ptrs.childNodes[1] = newLeaf;
                newRoot->numKeys = 1;
                cursor->parent = newRoot;
                newLeaf->parent = newRoot;
                tree->root = newRoot;
            }
            else
            {
                // If the split occurs below the root, update the parent node
                insertInternal(tree, parent, newLeaf->keys[0], newLeaf->names[0], newLeaf->departments[0], newLeaf);
            }

            printf("%d: INSERTED INTO LEAF NODE, SPLIT OCCURRED\n", key);
        }
    }
}





void insertInternal(BPTree *tree, Node *parent, int key, char *name, char *dept, Node *right)
{
    if (parent->numKeys < MAX_NODE)
    {
        // Insert the key, name, and department in the correct position while maintaining sorted order
        int i = parent->numKeys - 1;
        while (i >= 0 && key < parent->keys[i])
        {
            parent->keys[i + 1] = parent->keys[i];
            parent->names[i + 1] = parent->names[i];
            parent->departments[i + 1] = parent->departments[i];
            parent->ptrs.childNodes[i + 2] = parent->ptrs.childNodes[i + 1];
            i--;
        }
        parent->keys[i + 1] = key;
        parent->names[i + 1] = strdup(name);
        parent->departments[i + 1] = strdup(dept);
        parent->ptrs.childNodes[i + 2] = right;
        parent->numKeys++;
    }
    else
    {
        // Internal node is full, split it into two nodes
        Node *newInternal = createNode(0);
        int tempKeys[MAX_NODE + 1];
        char *tempNames[MAX_NODE + 1];
        char *tempDepts[MAX_NODE + 1];
        Node *tempPtrs[MAX_NODE + 2];
        for (int i = 0; i < MAX_NODE; i++)
        {
            tempKeys[i] = parent->keys[i];
            tempNames[i] = strdup(parent->names[i]);
            tempDepts[i] = strdup(parent->departments[i]);
            tempPtrs[i + 1] = parent->ptrs.childNodes[i + 1];
        }
        int i = MAX_NODE - 1;
        while (i >= 0 && key < tempKeys[i])
        {
            tempKeys[i + 1] = tempKeys[i];
            tempNames[i + 1] = tempNames[i];
            tempDepts[i + 1] = tempDepts[i];
            tempPtrs[i + 2] = tempPtrs[i + 1];
            i--;
        }
        tempKeys[i + 1] = key;
        tempNames[i + 1] = strdup(name);
        tempDepts[i + 1] = strdup(dept);
        tempPtrs[i + 2] = right;

        parent->numKeys = (MAX_NODE + 1) / 2;
        newInternal->numKeys = MAX_NODE - (MAX_NODE + 1) / 2;

        for (int j = 0; j < parent->numKeys; j++)
        {
            parent->keys[j] = tempKeys[j];
            parent->names[j] = tempNames[j];
            parent->departments[j] = tempDepts[j];
            parent->ptrs.childNodes[j + 1] = tempPtrs[j + 1];
        }
        for (int j = 0, k = parent->numKeys + 1; j < newInternal->numKeys; j++, k++)
        {
            newInternal->keys[j] = tempKeys[k];
            newInternal->names[j] = tempNames[k];
            newInternal->departments[j] = tempDepts[k];
            newInternal->ptrs.childNodes[j + 1] = tempPtrs[k];
        }

        if (parent == tree->root)
        {
            // If the split occurs at the root, create a new root
            Node *newRoot = createNode(0);
            newRoot->keys[0] = parent->keys[parent->numKeys];
            newRoot->names[0] = strdup(parent->names[parent->numKeys]);
            newRoot->departments[0] = strdup(parent->departments[parent->numKeys]);
            newRoot->ptrs.childNodes[0] = parent;
            newRoot->ptrs.childNodes[1] = newInternal;
            newRoot->numKeys = 1;
            parent->parent = newRoot;
            newInternal->parent = newRoot;
            tree->root = newRoot;
        }
        else
        {
            // If the split occurs below the root, update the parent node
            insertInternal(tree, parent->parent, parent->keys[parent->numKeys], parent->names[parent->numKeys], parent->departments[parent->numKeys], newInternal);
        }
    }
}









// -------------------------------searching function--------------------------------


// Get the root node of the B+ tree
Node *BPTree_getRoot(struct BPTree *tree)
{
    return tree->root;
}
/*
*/

void search(BPTree *bPTree)
{
    int searchOption;
    char searchParam[100];
    int searchValue;

    printf("What would you like to search by?\n");
    printf("1. Roll Number\n");
    printf("2. Name\n");
    printf("3. Department\n");
    printf("Enter your choice (1-3): ");
    scanf("%d", &searchOption);

    switch (searchOption)
    {
    case 1:
        printf("Enter the roll number to search: ");
        scanf("%d", &searchValue);
        break;
    case 2:
        printf("Enter the name to search: ");
        scanf("%s", searchParam);
        break;
    case 3:
        printf("Enter the department to search: ");
        scanf("%s", searchParam);
        break;
    default:
        printf("Invalid search option.\n");
        return;
    }

    Node *cursor = BPTree_getRoot(bPTree); // Start from the root node

    // Traverse the tree until reaching a leaf node
    while (!cursor->isLeaf)
    {
        int i;
        for (i = 0; i < cursor->numKeys; i++)
        {
            if (searchOption == 1 && searchValue < cursor->keys[i])
            {
                break;
            }
            else if (searchOption == 2 && strcmp(searchParam, cursor->names[i]) < 0)
            {
                break;
            }
            else if (searchOption == 3 && strcmp(searchParam, cursor->departments[i]) < 0)
            {
                break;
            }
        }
        cursor = cursor->ptrs.childNodes[i];
    }

    // Search for the records in the leaf node
    int found = 0;
    for (int i = 0; i < cursor->numKeys; i++)
    {
        if (searchOption == 1 && cursor->keys[i] == searchValue)
        {
            char fileName[100];
            sprintf(fileName, "DBFiles/%d.txt", cursor->keys[i]);
            FILE *filePtr = fopen(fileName, "r");
            if (filePtr == NULL)
            {
                printf("Error opening file for roll no %d.\n", cursor->keys[i]);
                continue;
            }

            char name[100], dpt[100];
            int age, marks;
            fscanf(filePtr, "%s %s %d %d", name, dpt, &age, &marks);
            printf("Roll No: %d\nName: %s\nDepartment: %s\nAge: %d\nMarks: %d\n", cursor->keys[i], name, dpt, age, marks);
            fclose(filePtr);
            found = 1;
        }
        else if (searchOption == 2 && strcmp(searchParam, cursor->names[i]) == 0)
        {
            char fileName[100];
            sprintf(fileName, "DBFiles/%d.txt", cursor->keys[i]);
            FILE *filePtr = fopen(fileName, "r");
            if (filePtr == NULL)
            {
                printf("Error opening file for roll no %d.\n", cursor->keys[i]);
                continue;
            }

            char name[100], dpt[100];
            int age, marks;
            fscanf(filePtr, "%s %s %d %d", name, dpt, &age, &marks);
            printf("Roll No: %d\nName: %s\nDepartment: %s\nAge: %d\nMarks: %d\n", cursor->keys[i], name, dpt, age, marks);
            fclose(filePtr);
            found = 1;
        }
        else if (searchOption == 3 && strcmp(searchParam, cursor->departments[i]) == 0)
        {
            char fileName[100];
            sprintf(fileName, "DBFiles/%d.txt", cursor->keys[i]);
            FILE *filePtr = fopen(fileName, "r");
            if (filePtr == NULL)
            {
                printf("Error opening file for roll no %d.\n", cursor->keys[i]);
                continue;
            }

            char name[100], dpt[100];
            int age, marks;
            fscanf(filePtr, "%s %s %d %d", name, dpt, &age, &marks);
            printf("Roll No: %d\nName: %s\nDepartment: %s\nAge: %d\nMarks: %d\n", cursor->keys[i], name, dpt, age, marks);
            fclose(filePtr);
            found = 1;
            printf("\n"); // Add a newline between records
        }
    }

    // Check if any records were found
    if (!found)
    {
        printf("No records found.\n");
    }
}

/*
void search(BPTree *bPTree)
{
    int searchOption;
    char searchParam[100];
    int searchValue;

    printf("What would you like to search by?\n");
    printf("1. Roll Number\n");
    printf("2. Name\n");
    printf("3. Department\n");
    printf("Enter your choice (1-3): ");
    scanf("%d", &searchOption);

    switch (searchOption)
    {
    case 1:
        printf("Enter the roll number to search: ");
        scanf("%d", &searchValue);
        break;
    case 2:
        printf("Enter the name to search: ");
        scanf("%s", searchParam);
        break;
    case 3:
        printf("Enter the department to search: ");
        scanf("%s", searchParam);
        break;
    default:
        printf("Invalid search option.\n");
        return;
    }

    Node *cursor = BPTree_getRoot(bPTree); // Start from the root node

    // Traverse the tree until reaching a leaf node
    while (!cursor->isLeaf)
    {
        int i;
        for (i = 0; i < cursor->numKeys; i++)
        {
            if (searchOption == 1 && searchValue < cursor->keys[i])
            {
                break;
            }
            else if (searchOption == 2 && strcmp(searchParam, cursor->names[i]) < 0)
            {
                break;
            }
            else if (searchOption == 3 && strcmp(searchParam, cursor->departments[i]) < 0)
            {
                break;
            }
        }
        cursor = cursor->ptrs.childNodes[i];
    }

    // Search for the records in the leaf node
    int found = 0;
    for (int i = 0; i < cursor->numKeys; i++)
    {
        if (searchOption == 1 && cursor->keys[i] == searchValue)
        {
            char fileName[100];
            sprintf(fileName, "DBFiles/%d.txt", cursor->keys[i]);
            FILE *filePtr = fopen(fileName, "r");
            if (filePtr == NULL)
            {
                printf("Error opening file for roll no %d.\n", cursor->keys[i]);
                continue;
            }

            char name[100], dpt[100];
            int age, marks;
            fscanf(filePtr, "%s %s %d %d", name, dpt, &age, &marks);
            printf("Roll No: %d\nName: %s\nDepartment: %s\nAge: %d\nMarks: %d\n", cursor->keys[i], name, dpt, age, marks);
            fclose(filePtr);
            found = 1;
        }
        else if (searchOption == 2 && strcmp(searchParam, cursor->names[i]) == 0)
        {
            char fileName[100];
            sprintf(fileName, "DBFiles/%d.txt", cursor->keys[i]);
            FILE *filePtr = fopen(fileName, "r");
            if (filePtr == NULL)
            {
                printf("Error opening file for roll no %d.\n", cursor->keys[i]);
                continue;
            }

            char name[100], dpt[100];
            int age, marks;
            fscanf(filePtr, "%s %s %d %d", name, dpt, &age, &marks);
            printf("Roll No: %d\nName: %s\nDepartment: %s\nAge: %d\nMarks: %d\n", cursor->keys[i], name, dpt, age, marks);
            fclose(filePtr);
            found = 1;
        }
        else if (searchOption == 3 && strcmp(searchParam, cursor->departments[i]) == 0)
        {
            char fileName[100];
            sprintf(fileName, "DBFiles/%d.txt", cursor->keys[i]);
            FILE *filePtr = fopen(fileName, "r");
            if (filePtr == NULL)
            {
                printf("Error opening file for roll no %d.\n", cursor->keys[i]);
                continue;
            }

            char name[100], dpt[100];
            int age, marks;
            fscanf(filePtr, "%s %s %d %d", name, dpt, &age, &marks);
            printf("Roll No: %d\nName: %s\nDepartment: %s\nAge: %d\nMarks: %d\n", cursor->keys[i], name, dpt, age, marks);
            fclose(filePtr);
            found = 1;
        }
    }

    // Check if any records were found
    if (!found)
    {
        printf("No records found.\n");
    }
}

*/

// -------------------------------deleting function--------------------------------


// Find the parent node of the given cursor node and child node
Node **BPTree_findParent(struct BPTree *tree, Node *cursor, Node *child)
{
    if (cursor == NULL || cursor->isLeaf)
        return NULL;

    int i;
    Node **parentPtr = NULL;

    for (i = 0; i < cursor->numKeys; i++)
    {
        if (child == cursor->ptrs.childNodes[i])
        {
            parentPtr = &cursor->ptrs.childNodes[i];
            break;
        }
        else if (child->keys[0] < cursor->keys[i])
        {
            parentPtr = BPTree_findParent(tree, cursor->ptrs.childNodes[i], child);
            break;
        }
    }

    if (parentPtr == NULL)
    {
        parentPtr = BPTree_findParent(tree, cursor->ptrs.childNodes[cursor->numKeys], child);
    }

    return parentPtr;
}

// Delete the key from the given node and rearrange its elements



Node *findParent(Node *root, Node *cursor)
{
    if (root == cursor)
        return NULL;

    Node *parent = NULL;
    for (int i = 0; i < root->numKeys; i++)
    {
        if (cursor->keys[0] < root->keys[i])
        {
            if (root->isLeaf)
                return NULL;
            else
            {
                parent = root;
                root = root->ptrs.childNodes[i];
                i = -1; // Resetting the index to 0
            }
        }
        else if (i == root->numKeys - 1)
        {
            if (root->isLeaf)
                return NULL;
            else
            {
                parent = root;
                root = root->ptrs.childNodes[i + 1];
                i = -1; // Resetting the index to 0
            }
        }
    }
    return parent;
}







//------------------------------------ display function -----------------------------

void display(Node *ptr, int blanks)
{
    if (ptr)
    {
        int i;

        if (ptr->isLeaf)
        {
            // Print the required number of blanks/spaces
            for (i = 1; i <= blanks; i++)
                printf(" ");

            // Print the keys, names, and departments in the leaf node
            for (i = 0; i < ptr->numKeys; i++)
                printf("%d (%s, %s) ", ptr->keys[i], ptr->names[i], ptr->departments[i]);
            printf("\n");
        }
        else
        {
            // Recursively display child nodes with increased blanks
            for (i = 0; i <= ptr->numKeys; i++)
                display(ptr->ptrs.childNodes[i], blanks + 10);

            // Print the required number of blanks/spaces
            for (i = 1; i <= blanks; i++)
                printf(" ");

            // Print the keys, names, and departments in the internal node
            for (i = 0; i < ptr->numKeys; i++)
                printf("%d (%s, %s) ", ptr->keys[i], ptr->names[i], ptr->departments[i]);
            printf("\n");
        }
    }
}

Node *findLeaf(Node *root, int key) {
    Node *cursor = root;

    // Traverse the tree until reaching a leaf node
    while (cursor && !cursor->isLeaf) {
        int i = 0;
        while (i < cursor->numKeys && key >= cursor->keys[i]) {
            i++;
        }
        cursor = cursor->ptrs.childNodes[i];
    }

    return cursor;
}

Node *getSibling(Node *node, int index) {
    if (!node->parent)
        return NULL;

    if (index == 0 && node->parent->numKeys > 1)
        return node->parent->ptrs.childNodes[1];
    else if (index == node->parent->numKeys && node->parent->numKeys > 1)
        return node->parent->ptrs.childNodes[index - 1];
    else if (index > 0 && index < node->parent->numKeys)
        return node->parent->ptrs.childNodes[index + 1];
    else
        return NULL;
}


// Function to get the index of a child node within its parent
int getParentIndex(Node *parent, Node *child) {
    for (int i = 0; i <= parent->numKeys; i++) {
        if (parent->ptrs.childNodes[i] == child) {
            return i;
        }
    }
    return -1; // Not found
}

void borrowFromSibling(Node *node, Node *sibling, int index)
{
    if (index < node->parent->numKeys && sibling->numKeys > (MAX_NODE + 1) / 2)
    {
        // Borrow a key, name, and department from the right sibling
        insertAtEnd(node, node->parent->keys[index], node->parent->names[index], node->parent->departments[index]);
        node->parent->keys[index] = sibling->keys[0];
        free(node->parent->names[index]);
        node->parent->names[index] = strdup(sibling->names[0]);
        free(node->parent->departments[index]);
        node->parent->departments[index] = strdup(sibling->departments[0]);
        node->numKeys++;
        sibling->numKeys--;

        // Adjust keys, names, departments, and pointers in the sibling
        for (int i = 0; i < sibling->numKeys; i++)
        {
            sibling->keys[i] = sibling->keys[i + 1];
            free(sibling->names[i]);
            sibling->names[i] = strdup(sibling->names[i + 1]);
            free(sibling->departments[i]);
            sibling->departments[i] = strdup(sibling->departments[i + 1]);
            sibling->ptrs.dataPtr[i] = sibling->ptrs.dataPtr[i + 1];
        }
        sibling->ptrs.dataPtr[sibling->numKeys] = sibling->ptrs.dataPtr[sibling->numKeys + 1];
    }
    else if (index > 0 && sibling->numKeys > (MAX_NODE + 1) / 2)
    {
        // Borrow a key, name, and department from the left sibling
        insertAtBeginning(node, node->parent->keys[index - 1], node->parent->names[index - 1], node->parent->departments[index - 1]);
        node->parent->keys[index - 1] = sibling->keys[sibling->numKeys - 1];
        free(node->parent->names[index - 1]);
        node->parent->names[index - 1] = strdup(sibling->names[sibling->numKeys - 1]);
        free(node->parent->departments[index - 1]);
        node->parent->departments[index - 1] = strdup(sibling->departments[sibling->numKeys - 1]);
        node->numKeys++;
        sibling->numKeys--;

        // Adjust keys, names, departments, and pointers in the sibling
        sibling->ptrs.dataPtr[sibling->numKeys] = sibling->ptrs.dataPtr[sibling->numKeys - 1];
    }
}



void mergeWithSibling(Node *root, Node *node, Node *sibling, int index)
{
    // Merge node with sibling
    insertAtEnd(node, node->parent->keys[index], node->parent->names[index], node->parent->departments[index]);
    for (int i = 0; i < sibling->numKeys; i++)
    {
        insertAtEnd(node, sibling->keys[i], sibling->names[i], sibling->departments[i]);
    }

    // Adjust pointers
    if (!node->isLeaf)
    {
        for (int i = 0; i <= sibling->numKeys; i++)
        {
            node->ptrs.childNodes[node->numKeys + i] = sibling->ptrs.childNodes[i];
            node->ptrs.childNodes[node->numKeys + i]->parent = node;
        }
    }

    // Adjust parent keys, names, departments, and pointers
    node->parent->numKeys--;
    for (int i = index; i < node->parent->numKeys; i++)
    {
        node->parent->keys[i] = node->parent->keys[i + 1];
        free(node->parent->names[i]);
        node->parent->names[i] = strdup(node->parent->names[i + 1]);
        free(node->parent->departments[i]);
        node->parent->departments[i] = strdup(node->parent->departments[i + 1]);
        node->parent->ptrs.childNodes[i + 1] = node->parent->ptrs.childNodes[i + 2];
    }

    // Free the memory allocated for the sibling node
    for (int i = 0; i < sibling->numKeys; i++)
    {
        free(sibling->names[i]);
        free(sibling->departments[i]);
    }
    free(sibling->names);
    free(sibling->departments);
    free(sibling);

    // If parent becomes empty, update root
    if (node->parent->numKeys == 0)
    {
        if (node->parent == root)
        {
            // Update root pointer
            root = node;
        }
        free(node->parent);
        node->parent = NULL;
    }
}

void removeKey(Node *root, int x)
{
    if (!root) return;

    // Find the leaf node containing the key
    Node *leaf = findLeaf(root, x);
    if (!leaf) {
        printf("Key %d not found in the tree.\n", x);
        return;
    }

    // Find the index of the key in the leaf node
    int index = -1;
    for (int i = 0; i < leaf->numKeys; i++) {
        if (leaf->keys[i] == x) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        printf("Key %d not found in the tree.\n", x);
        return;
    }

    // Remove the key, name, and department from the leaf node
    for (int i = index; i < leaf->numKeys - 1; i++) {
        leaf->keys[i] = leaf->keys[i + 1];
        free(leaf->names[i]);
        leaf->names[i] = strdup(leaf->names[i + 1]);
        free(leaf->departments[i]);
        leaf->departments[i] = strdup(leaf->departments[i + 1]);
        leaf->ptrs.dataPtr[i] = leaf->ptrs.dataPtr[i + 1];
    }
    free(leaf->names[leaf->numKeys - 1]);
    free(leaf->departments[leaf->numKeys - 1]);
    leaf->numKeys--;

    // Adjust parent pointers
    Node *parent = findParent(root, leaf);
    while (parent && index == parent->numKeys) {
        index = getParentIndex(parent, leaf);
        leaf = parent;
        parent = findParent(root, leaf);
    }

    // Handle underflow if necessary
    if (leaf->numKeys < (MAX_NODE + 1) / 2) {
        Node *sibling = getSibling(leaf, index);
        if (sibling && sibling->numKeys > (MAX_NODE + 1) / 2) {
            borrowFromSibling(leaf, sibling, index);
        } else {
            mergeWithSibling(root, leaf, sibling, index);
        }
    }
}





void insertAtEnd(Node *node, int key, char *name, char *dept)
{
    node->keys[node->numKeys] = key;
    node->departments[node->numKeys] = strdup(dept);
    node->numKeys++;
}

void insertAtBeginning(Node *node, int key, char *name, char *dept)
{
    for (int i = node->numKeys; i > 0; i--)
    {
        node->keys[i] = node->keys[i - 1];
        free(node->names[i]);
        node->names[i] = strdup(node->names[i - 1]);
        free(node->departments[i]);
        node->departments[i] = strdup(node->departments[i - 1]);
    }
    node->keys[0] = key;
    node->names[0] = strdup(name);
    node->departments[0] = strdup(dept);
    node->numKeys++;
}


