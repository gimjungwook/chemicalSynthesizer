#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define MAX_NODES 100
#define MAX_EDGES 1000
#define MAX_LABEL_LEN 10
#define MAX_COND_LEN 32
#define MAX_INV 100
#define INF INT_MAX

typedef struct
{
    char label[MAX_LABEL_LEN];
    int id;
    int isUsed;
} Node;

typedef struct
{
    int id; // 고유 식별자
    int from;
    int to;
    int weight;
    char condition[MAX_COND_LEN];
} Edge;

typedef struct
{
    Node nodes[MAX_NODES];
    Edge edges[MAX_EDGES];
    int nodeCount;
    int edgeCount;
    int nextEdgeId;
    char inventory[MAX_INV][MAX_COND_LEN];
    int inventoryCount;

    int highlightEdgeIds[MAX_EDGES];
    int highlightCount;
} Graph;

Graph *qsortGraphContext = NULL;

void trimNewline(char *str)
{
    str[strcspn(str, "\r\n")] = 0;
}

int getNodeId(Graph *g, const char *label)
{
    for (int i = 0; i < g->nodeCount; ++i)
    {
        if (g->nodes[i].isUsed && strcmp(g->nodes[i].label, label) == 0)
            return g->nodes[i].id;
    }
    return -1;
}

int conditionInInventory(Graph *g, const char *cond)
{
    if (cond == NULL || strlen(cond) == 0)
        return 1;

    char orCopy[MAX_COND_LEN];
    strncpy(orCopy, cond, MAX_COND_LEN - 1);
    orCopy[MAX_COND_LEN - 1] = '\0';

    char *orToken = strtok(orCopy, "|");
    while (orToken != NULL)
    {
        int andSatisfied = 1;
        char andCopy[MAX_COND_LEN];
        strncpy(andCopy, orToken, MAX_COND_LEN - 1);
        andCopy[MAX_COND_LEN - 1] = '\0';

        char *andToken = strtok(andCopy, "&");
        while (andToken != NULL)
        {
            while (*andToken == ' ')
                andToken++;
            char *end = andToken + strlen(andToken) - 1;
            while (end > andToken && *end == ' ')
                *end-- = '\0';

            int found = 0;
            for (int i = 0; i < g->inventoryCount; ++i)
            {
                if (strcmp(g->inventory[i], andToken) == 0)
                {
                    found = 1;
                    break;
                }
            }
            if (!found)
            {
                andSatisfied = 0;
                break;
            }
            andToken = strtok(NULL, "&");
        }

        if (andSatisfied)
            return 1;

        orToken = strtok(NULL, "|");
    }

    return 0;
}
void addNode(Graph *g, const char *label)
{
    if (getNodeId(g, label) != -1)
    {
        printf("⚠️ Node '%s' already exists.\n", label);
        return;
    }
    for (int i = 0; i < g->nodeCount; ++i)
    {
        if (!g->nodes[i].isUsed)
        {
            strcpy(g->nodes[i].label, label);
            g->nodes[i].isUsed = 1;
            printf("✅ Node '%s' added (reused slot).\n", label);
            return;
        }
    }
    if (g->nodeCount >= MAX_NODES)
    {
        printf("❌ Maximum number of nodes reached.\n");
        return;
    }
    strcpy(g->nodes[g->nodeCount].label, label);
    g->nodes[g->nodeCount].id = g->nodeCount;
    g->nodes[g->nodeCount].isUsed = 1;
    g->nodeCount++;
    printf("✅ Node '%s' added.\n", label);
}

void addEdge(Graph *g, const char *fromLabel, const char *toLabel, int weight, const char *condition)
{
    int fromId = getNodeId(g, fromLabel);
    int toId = getNodeId(g, toLabel);
    if (fromId == -1 || toId == -1)
    {
        printf("❌ One or both nodes not found.\n");
        return;
    }
    if (g->edgeCount + 1 > MAX_EDGES)
    {
        printf("❌ Maximum number of edges reached.\n");
        return;
    }

    Edge e;
    e.id = g->nextEdgeId++;
    e.from = fromId;
    e.to = toId;
    e.weight = weight;
    if (condition)
    {
        strncpy(e.condition, condition, MAX_COND_LEN - 1);
        e.condition[MAX_COND_LEN - 1] = '\0';
    }
    else
    {
        e.condition[0] = '\0';
    }
    g->edges[g->edgeCount++] = e;

    printf("✅ Edge added: %s → %s (w=%d", fromLabel, toLabel, weight);
    if (e.condition[0] != '\0')
    {
        printf(", condition:%s", e.condition);
    }
    printf(")\n");
}

void removeNode(Graph *g, const char *label)
{
    int id = getNodeId(g, label);
    if (id == -1)
    {
        printf("❌ Node '%s' not found.\n", label);
        return;
    }
    g->nodes[id].isUsed = 0;

    int k = 0;
    for (int i = 0; i < g->edgeCount; ++i)
    {
        if (g->edges[i].from != id && g->edges[i].to != id)
        {
            g->edges[k++] = g->edges[i];
        }
    }
    g->edgeCount = k;

    printf("✅ Node '%s' and its edges removed.\n", label);
}

void updateNode(Graph *g, const char *oldLabel, const char *newLabel)
{
    int id = getNodeId(g, oldLabel);
    if (id == -1 || getNodeId(g, newLabel) != -1)
    {
        printf("❌ Rename failed.\n");
        return;
    }
    strcpy(g->nodes[id].label, newLabel);
    printf("✅ Node '%s' renamed to '%s'.\n", oldLabel, newLabel);
}

void removeEdge(Graph *g, const char *fromLabel, const char *toLabel)
{
    int fromId = getNodeId(g, fromLabel);
    int toId = getNodeId(g, toLabel);
    if (fromId == -1 || toId == -1)
    {
        printf("❌ Node not found.\n");
        return;
    }
    int k = 0;
    for (int i = 0; i < g->edgeCount; ++i)
    {
        if (!(g->edges[i].from == fromId && g->edges[i].to == toId))
        {
            g->edges[k++] = g->edges[i];
        }
    }
    g->edgeCount = k;
    printf("✅ Edge(s) removed: %s → %s\n", fromLabel, toLabel);
}
// 목적지 라벨 기준으로 정렬
int compareEdgesByDestLabel(const void *a, const void *b)
{
    Edge *ea = *(Edge **)a;
    Edge *eb = *(Edge **)b;
    const char *la = qsortGraphContext->nodes[ea->to].label;
    const char *lb = qsortGraphContext->nodes[eb->to].label;
    return strcmp(la, lb);
}

void ShowGraph(Graph *g)
{
    printf("\n📋 Adjacency List:\n");
    for (int i = 0; i < g->nodeCount; ++i)
    {
        if (!g->nodes[i].isUsed)
            continue;

        printf("%s → ", g->nodes[i].label);
        int id = g->nodes[i].id;

        Edge *outEdges[MAX_EDGES];
        int outCount = 0;

        for (int j = 0; j < g->edgeCount; ++j)
        {
            if (g->edges[j].from == id && g->nodes[g->edges[j].to].isUsed)
            {
                outEdges[outCount++] = &g->edges[j];
            }
        }

        qsortGraphContext = g; // 전역 변수 설정
        qsort(outEdges, outCount, sizeof(Edge *), compareEdgesByDestLabel);

        for (int j = 0; j < outCount; ++j)
        {
            Edge *e = outEdges[j];
            printf("%s(%d", g->nodes[e->to].label, e->weight);
            if (strlen(e->condition) > 0)
                printf(":%s", e->condition);
            printf(") ");
        }

        printf("\n");
    }
}

void addCondition(Graph *g, const char *cond)
{
    if (g->inventoryCount >= MAX_INV)
    {
        printf("❌ Inventory is full.\n");
        return;
    }
    for (int i = 0; i < g->inventoryCount; ++i)
    {
        if (strcmp(g->inventory[i], cond) == 0)
        {
            printf("⚠️ Condition '%s' already in inventory.\n", cond);
            return;
        }
    }
    strncpy(g->inventory[g->inventoryCount], cond, MAX_COND_LEN - 1);
    g->inventory[g->inventoryCount][MAX_COND_LEN - 1] = '\0';
    g->inventoryCount++;
    printf("✅ Condition '%s' added to inventory.\n", cond);
}

void removeCondition(Graph *g, const char *cond)
{
    int k = 0;
    int found = 0;
    for (int i = 0; i < g->inventoryCount; ++i)
    {
        if (strcmp(g->inventory[i], cond) == 0)
        {
            found = 1;
            continue;
        }
        strncpy(g->inventory[k++], g->inventory[i], MAX_COND_LEN);
    }
    if (!found)
    {
        printf("❌ Condition '%s' not in inventory.\n", cond);
        return;
    }
    g->inventoryCount = k;
    printf("✅ Condition '%s' removed from inventory.\n", cond);
}

void showInventory(Graph *g)
{
    printf("\n🎒 Inventory:\n");
    if (g->inventoryCount == 0)
    {
        printf("  (empty)\n");
        return;
    }
    for (int i = 0; i < g->inventoryCount; ++i)
    {
        printf("  %s\n", g->inventory[i]);
    }
}
int isEdgeHighlighted(Graph *g, int edgeId)
{
    for (int i = 0; i < g->highlightCount; ++i)
    {
        if (g->highlightEdgeIds[i] == edgeId)
            return 1;
    }
    return 0;
}

void synthesize(Graph *g, const char *startLabel, const char *goalLabel)
{
    int startId = getNodeId(g, startLabel);
    int goalId = getNodeId(g, goalLabel);
    if (startId == -1 || goalId == -1)
    {
        printf("❌ One or both nodes not found.\n");
        return;
    }

    int dist[MAX_NODES], visited[MAX_NODES], prev[MAX_NODES];
    for (int i = 0; i < g->nodeCount; ++i)
    {
        dist[i] = INF;
        visited[i] = 0;
        prev[i] = -1;
    }
    dist[startId] = 0;
    g->highlightCount = 0;

    printf("\n🔍 Beginning synthesis from %s to %s with inventory:\n", startLabel, goalLabel);
    showInventory(g);
    printf("\n");

    for (int iter = 0; iter < g->nodeCount; ++iter)
    {
        int u = -1, minDist = INF;
        for (int i = 0; i < g->nodeCount; ++i)
        {
            if (g->nodes[i].isUsed && !visited[i] && dist[i] < minDist)
            {
                minDist = dist[i];
                u = i;
            }
        }
        if (u == -1 || u == goalId)
            break;
        visited[u] = 1;

        printf("⏱️ Visiting node %s (dist: %d)\n", g->nodes[u].label, dist[u]);

        for (int e = 0; e < g->edgeCount; ++e)
        {
            if (g->edges[e].from != u)
                continue;
            int v = g->edges[e].to;
            if (!g->nodes[v].isUsed)
                continue;

            int w = g->edges[e].weight;
            char condBuff[MAX_COND_LEN] = "";
            if (strlen(g->edges[e].condition) > 0)
                strncpy(condBuff, g->edges[e].condition, MAX_COND_LEN - 1);

            // 조건이 있는 경우, inventory에 없으면 skip
            if (strlen(condBuff) > 0 && !conditionInInventory(g, condBuff))
            {
                printf("  🚫 Skipping %s → %s (cost: %d, condition: %s) - not in inventory\n",
                       g->nodes[u].label, g->nodes[v].label, w, condBuff);
                continue;
            }

            int effectiveWeight = w; // 조건 충족하면 패널티 없음

            printf("  👍 Considering %s → %s (cost: %d", g->nodes[u].label, g->nodes[v].label, effectiveWeight);
            if (strlen(condBuff) > 0)
                printf(", condition: %s", condBuff);
            printf(")\n");

            if (dist[u] + effectiveWeight < dist[v])
            {
                dist[v] = dist[u] + effectiveWeight;
                prev[v] = u;
                printf("    🔄 Updated dist[%s] to %d\n", g->nodes[v].label, dist[v]);
            }
        }
        printf("\n");
    }

    if (dist[goalId] == INF)
    {
        printf("❌ No viable path from %s to %s.\n", startLabel, goalLabel);
        return;
    }

    // 경로 역추적
    int path[MAX_NODES], pathLen = 0, cur = goalId;
    while (cur != -1)
    {
        path[pathLen++] = cur;
        cur = prev[cur];
    }
    for (int i = 0; i < pathLen / 2; ++i)
    {
        int tmp = path[i];
        path[i] = path[pathLen - 1 - i];
        path[pathLen - 1 - i] = tmp;
    }

    g->highlightCount = 0;
    printf("🧪 Synthesis Path from %s to %s:\n", startLabel, goalLabel);
    for (int i = 0; i < pathLen - 1; ++i)
    {
        int u = path[i], v = path[i + 1];

        for (int e = 0; e < g->edgeCount; ++e)
        {
            if (g->edges[e].from == u && g->edges[e].to == v)
            {
                int w = g->edges[e].weight;
                char cond[MAX_COND_LEN] = "";
                if (strlen(g->edges[e].condition) > 0)
                    strncpy(cond, g->edges[e].condition, MAX_COND_LEN - 1);

                if (strlen(cond) == 0 || conditionInInventory(g, cond))
                {
                    if (dist[u] + w == dist[v])
                    {
                        g->highlightEdgeIds[g->highlightCount++] = g->edges[e].id;
                        if (strlen(cond) > 0)
                            printf("  %s → %s  (cost: %d, condition: %s)\n", g->nodes[u].label, g->nodes[v].label, w, cond);
                        else
                            printf("  %s → %s  (cost: %d)\n", g->nodes[u].label, g->nodes[v].label, w);
                        break;
                    }
                }
            }
        }
    }
    printf("🧮 Total Cost: %d\n", dist[goalId]);
}

void visualizeGraphToFile(Graph *g, const char *basename)
{
    char dotFilename[512], pngFilename[512], cmd[1024];
    snprintf(dotFilename, sizeof(dotFilename), "%s.dot", basename);
    FILE *file = fopen(dotFilename, "w");
    if (!file)
    {
        printf("❌ Failed to open DOT file: %s\n", dotFilename);
        return;
    }

    fprintf(file, "digraph G {\n");

    // 노드 출력
    for (int i = 0; i < g->nodeCount; ++i)
    {
        if (g->nodes[i].isUsed)
            fprintf(file, "    \"%s\";\n", g->nodes[i].label);
    }

    // 엣지 출력
    for (int i = 0; i < g->edgeCount; ++i)
    {
        int u = g->edges[i].from, v = g->edges[i].to;
        if (!g->nodes[u].isUsed || !g->nodes[v].isUsed)
            continue;

        char condBuff[MAX_COND_LEN] = "";
        if (strlen(g->edges[i].condition) > 0)
        {
            strncpy(condBuff, g->edges[i].condition, MAX_COND_LEN - 1);
            condBuff[MAX_COND_LEN - 1] = '\0';
        }

        // 기본 스타일
        char *color = "black";
        char *style = "solid";
        int penwidth = 1;

        // 조건 존재 여부로 색상 변경
        if (strlen(condBuff) > 0)
        {
            if (conditionInInventory(g, condBuff))
                color = "green";
            else
            {
                color = "gray";
                style = "dashed";
            }
        }

        // synthesize 경로 강조
        if (isEdgeHighlighted(g, g->edges[i].id))
        {
            color = "blue";
            penwidth = 3;
        }

        // DOT 라벨 출력
        if (strlen(g->edges[i].condition) > 0)
        {
            fprintf(file,
                    "    \"%s\" -> \"%s\" [label=\"%d,%s\", color=%s, style=%s, penwidth=%d];\n",
                    g->nodes[u].label, g->nodes[v].label,
                    g->edges[i].weight, g->edges[i].condition,
                    color, style, penwidth);
        }
        else
        {
            fprintf(file,
                    "    \"%s\" -> \"%s\" [label=\"%d\", color=%s, style=%s, penwidth=%d];\n",
                    g->nodes[u].label, g->nodes[v].label,
                    g->edges[i].weight,
                    color, style, penwidth);
        }
    }

    fprintf(file, "}\n");
    fclose(file);

    snprintf(pngFilename, sizeof(pngFilename), "img/%s.png", basename);
    snprintf(cmd, sizeof(cmd), "dot -Tpng %s -o %s", dotFilename, pngFilename);
    int res = system(cmd);
    if (res == -1)
    {
        printf("❌ Failed to run dot command.\n");
    }
    else
    {
        printf("✅ Generated PNG: img/%s.png\n", basename);
    }

    remove(dotFilename);
}

int main()
{
    Graph g;
    char command[256];
    memset(&g, 0, sizeof(Graph));
    g.nextEdgeId = 1;

    printf("✅ Started with an empty graph. Use 'load <filename>' to load from CSV.\n");
    printf("ℹ️ Type 'help' for instructions.\n");

    while (1)
    {
        printf("\n> ");
        if (!fgets(command, sizeof(command), stdin))
            break;
        trimNewline(command);

        if (strcmp(command, "exit") == 0)
            break;
        else if (strcmp(command, "help") == 0)
        {
            printf("\n🛠️  Commands:\n");
            printf("  help                                 - Show this help message\n");
            printf("  addNode <label>                      - Add a node\n");
            printf("  removeNode <label>                   - Remove a node\n");
            printf("  updateNode <old> <new>               - Rename a node\n");
            printf("  addEdge <from> <to> <w>,<cond>       - Add edge with weight & condition\n");
            printf("  removeEdge <from> <to>               - Remove all edges from→to\n");
            printf("  addCondition <cond>                  - Add condition to inventory\n");
            printf("  removeCondition <cond>               - Remove condition from inventory\n");
            printf("  showInventory                        - List all current conditions\n");
            printf("  show                                 - Show graph (sorted)\n");
            printf("  synthesize <start> <goal>            - Find best synthesis path\n");
            printf("  visualize <name>                     - Export .png of graph\n");
            printf("  exit                                 - Exit the program\n");
        }
        else if (strncmp(command, "addNode ", 8) == 0)
        {
            char label[32];
            sscanf(command + 8, "%s", label);
            addNode(&g, label);
        }
        else if (strncmp(command, "removeNode ", 11) == 0)
        {
            char label[32];
            sscanf(command + 11, "%s", label);
            removeNode(&g, label);
        }
        else if (strncmp(command, "updateNode ", 11) == 0)
        {
            char oldL[32], newL[32];
            sscanf(command + 11, "%s %s", oldL, newL);
            updateNode(&g, oldL, newL);
        }
        else if (strncmp(command, "addEdge ", 8) == 0)
        {
            char from[32], to[32], rest[64];
            sscanf(command + 8, "%s %s %s", from, to, rest);
            char *comma = strchr(rest, ',');
            if (!comma)
            {
                printf("❌ Usage: addEdge <from> <to> <weight>,<condition>\n");
                continue;
            }
            *comma = '\0';
            int w = atoi(rest);
            char cond[MAX_COND_LEN] = "";
            if (*(comma + 1) != '\0')
            {
                strncpy(cond, comma + 1, MAX_COND_LEN - 1);
                cond[MAX_COND_LEN - 1] = '\0';
            }
            addEdge(&g, from, to, w, cond);
        }
        else if (strncmp(command, "removeEdge ", 11) == 0)
        {
            char from[32], to[32];
            sscanf(command + 11, "%s %s", from, to);
            removeEdge(&g, from, to);
        }
        else if (strncmp(command, "addCondition ", 13) == 0)
        {
            char cond[32];
            sscanf(command + 13, "%s", cond);
            addCondition(&g, cond);
        }
        else if (strncmp(command, "removeCondition ", 16) == 0)
        {
            char cond[32];
            sscanf(command + 16, "%s", cond);
            removeCondition(&g, cond);
        }
        else if (strcmp(command, "showInventory") == 0)
        {
            showInventory(&g);
        }
        else if (strcmp(command, "show") == 0)
        {
            ShowGraph(&g);
        }
        else if (strncmp(command, "synthesize ", 11) == 0)
        {
            char start[32], goal[32];
            sscanf(command + 11, "%s %s", start, goal);
            synthesize(&g, start, goal);
        }
        else if (strncmp(command, "visualize ", 10) == 0)
        {
            char basename[128];
            sscanf(command + 10, "%s", basename);
            visualizeGraphToFile(&g, basename);
        }
        else if (strncmp(command, "print", 5) == 0)
        {
            char content[128];
            sscanf(command + 6, "%s", content);
            printf("\n\n\n\n\n%s", content);
        }
        else
        {
            printf("❓ Unknown command. Type 'help' for a list of commands.\n");
        }
    }

    printf("👋 Goodbye!\n");
    return 0;
}