#ifndef QRL_H
#define QRL_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define BALL_POS_BINS 12
#define PADDLE_POS_BINS 12
#define ACTION_SPACE_SIZE 3
#define LEARNING_RATE 0.1
#define DISCOUNT_FACTOR 0.9
#define EXPLORATION_PROBABILITY 0.1
#define EPISODES 1000

struct QLearning
{
    // 0 = failed, 1 = hit 2 = right direction (states)
    // 0 = left, 1 = stay, 2 = right (actions)
    //double q_matrix[2][3]; // states, actions
    //int mode; // 1 = Exploration, 2 = Exploitation
    //int state;
    //int last_move;
    //int times;
    double qTable[BALL_POS_BINS][PADDLE_POS_BINS][ACTION_SPACE_SIZE];
};


void printQTable(struct QLearning* q) {
    printf("Q-Table:\n");
    printf("| BallPos | PaddlePos | Action 0 | Action 1 | Action 2 |\n");
    printf("|---------|-----------|----------|----------|----------|\n");

    for (int ballPos = 0; ballPos < BALL_POS_BINS; ++ballPos) {
        for (int paddlePos = 0; paddlePos < PADDLE_POS_BINS; ++paddlePos) {
            printf("| %-7d | %-9d |", ballPos, paddlePos);

            for (int action = 0; action < ACTION_SPACE_SIZE; ++action) {
                printf(" %-8.2f |", q->qTable[ballPos][paddlePos][action]);
            }

            printf("\n");
        }
    }
}


struct QLearning qrl_init()
{
    srand(time(NULL));
    struct QLearning q;
    for (int i = 0; i < BALL_POS_BINS; ++i) {
        for (int j = 0; j < PADDLE_POS_BINS; ++j) {
            for (int k = 0; k < ACTION_SPACE_SIZE; ++k) {
                q.qTable[i][j][k] = 0.0;
            }
        }
    }
    // 0 = ball.x, 1 = paddle.x
    //q.q_matrix[0][0] = 0;
    //q.q_matrix[0][1] = 0;
    //q.q_matrix[0][2] = 0;
    //q.q_matrix[1][0] = 0;
    //q.q_matrix[1][1] = 0;
    //q.q_matrix[1][2] = 0;
    ////q.q_matrix[2][0] = 0;
    ////q.q_matrix[2][1] = 0;
    ////q.q_matrix[2][2] = 0;
    //q.mode = 1;
    //q.state = 0;
    //q.times = 0;
    //q.last_move = 0;
    return q;
}


double max_3(double a, double b, double c)
{
    return a > b ? (a > c ? a : c) : (b > c ? b : c);
}


int chooseAction(struct QLearning* q, int ballPos, int paddlePos) {
    //(double)rand() / RAND_MAX < EXPLORATION_PROBABILITY
    static int i = 0;
    if (i < 100000) {
        int v = rand() % ACTION_SPACE_SIZE;
        //printf("%d\n", v);
        ++i;
        return v;
    }
    else {
        if (i == 100000)
        {
            printQTable(q);
            ++i;
        }

        const double ba1 = q->qTable[ballPos][paddlePos][0];
        const double ba2 = q->qTable[ballPos][paddlePos][1];
        const double ba3 = q->qTable[ballPos][paddlePos][2];
        if (ba1 == ba2 && ba2 == ba3)
        {
            return rand() % ACTION_SPACE_SIZE;
        }

        int bestAction = 0;
        for (int k = 1; k < ACTION_SPACE_SIZE; ++k)
        {
            if (q->qTable[ballPos][paddlePos][k] > q->qTable[ballPos][paddlePos][bestAction]) {
                bestAction = k;
            }
        }
        return bestAction;
    }
}


void updateQValue(struct QLearning* q, int ballPos, int paddlePos, int action, int nextBallPos, int nextPaddlePos, double reward)
{
    double maxQValue = q->qTable[nextBallPos][nextPaddlePos][0];
    for (int i = 1; i < ACTION_SPACE_SIZE; ++i) {
        if (q->qTable[nextBallPos][nextPaddlePos][i] > maxQValue) {
            maxQValue = q->qTable[nextBallPos][nextPaddlePos][i];
        }
    }

    q->qTable[ballPos][paddlePos][action] =
        (1 - LEARNING_RATE) * q->qTable[ballPos][paddlePos][action] +
        LEARNING_RATE * (reward + DISCOUNT_FACTOR * maxQValue);
}



#endif // QRL_H