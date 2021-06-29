#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <queue>
#include <limits.h>
#include <algorithm>
#include <iostream>
struct Point {
    int x, y;
	Point() : Point(0, 0) {}
	Point(int x, int y) : x(x), y(y) {}
	bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
};

int player;
const int SIZE = 8;
std::array<std::array<int, SIZE>, SIZE> cur_board;
std::vector<Point> cur_next_valid_spots;
std::array<int, 3> cur_disc_count;
std::vector<Point> cur_x_spots;

class State{
    private:
        int get_next_player(int player) const {
            return 3 - player;
        }
        bool is_spot_on_board(Point p) const {
            return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
        }
        int get_disc(Point p) const {
            return board[p.x][p.y];
        }
        void set_disc(Point p, int disc) {
            board[p.x][p.y] = disc;
        }
        bool is_disc_at(Point p, int disc) const {
            if (!is_spot_on_board(p))
                return false;
            if (get_disc(p) != disc)
                return false;
            return true;
        }
        bool is_spot_valid(Point center) const {
            if (get_disc(center) != EMPTY)
                return false;
            for (Point dir: directions) {
                // Move along the direction while testing.
                Point p = center + dir;
                if (!is_disc_at(p, get_next_player(cur_player)))
                    continue;
                p = p + dir;
                while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                    if (is_disc_at(p, cur_player))
                        return true;
                    p = p + dir;
                }
            }
            return false;
        }
        void flip_discs(Point center) {
            for (Point dir: directions) {
                // Move along the direction while testing.
                Point p = center + dir;
                if (!is_disc_at(p, get_next_player(cur_player)))
                    continue;
                std::vector<Point> discs({p});
                p = p + dir;
                while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                    if (is_disc_at(p, cur_player)) {
                        for (Point s: discs) {
                            set_disc(s, cur_player);
                        }
                        disc_count[cur_player] += discs.size();
                        disc_count[get_next_player(cur_player)] -= discs.size();
                        break;
                    }
                    discs.push_back(p);
                    p = p + dir;
                }
            }
        }
    public:
        enum SPOT_STATE {
            EMPTY = 0,
            BLACK = 1,
            WHITE = 2
        };
        static const int SIZE = 8;
        const std::array<Point, 8> directions{{
            Point(-1, -1), Point(-1, 0), Point(-1, 1),
            Point(0, -1), /*{0, 0}, */Point(0, 1),
            Point(1, -1), Point(1, 0), Point(1, 1)
        }};
        std::array<std::array<int, SIZE>, SIZE> board;
        std::vector<Point> next_valid_spots;
        std::array<int, 3> disc_count;
        int cur_player;
        bool done;
        int winner;
        int heuristic;
        State(std::vector<Point> &v = cur_next_valid_spots, std::array<int, 3>&a = cur_disc_count)
        :next_valid_spots(v),disc_count(a),done(false),winner(-1){
            cur_player = player;
            board = cur_board;
            setheuristic();
        }
        State(const State& s){
            board = s.board;
            next_valid_spots = s.next_valid_spots;
            disc_count = s.disc_count;
            cur_player = s.cur_player;
            done = s.done;
            winner = s.winner;
            heuristic = s.heuristic;
        }
        int evaluate_unbalanced_edge(){
            int weak_side=0;
            if(board[0][0]!=player&&board[0][7]!=player){
                int disc = 0;
                for(int i = 1 ; i <= 6 ; i++){
                    if(board[0][i]==player)disc++;
                }
                if(disc!=6)weak_side++;
            }
            else if(board[7][7]!=player&&board[0][7]!=player){
                int disc = 0;
                for(int i = 1 ; i <= 6 ; i++){
                    if(board[i][7]==player)disc++;
                }
                if(disc!=6)weak_side++;
            }
            else if(board[7][0]!=player&&board[7][7]!=player){
                int disc = 0;
                for(int i = 1 ; i <= 6 ; i++){
                    if(board[7][i]==player)disc++;
                }
                if(disc!=6)weak_side++;
            }
            else if(board[0][0]!=player&&board[7][0]!=player){
                int disc = 0;
                for(int i = 1 ; i <= 6 ; i++){
                    if(board[i][0]==player)disc++;
                }
                if(disc!=6)weak_side++;
            }
            return weak_side;
        }
        int evaluate_determined(){
            int exact = 0;
            if(board[0][0]==player){
                exact+=1;
                for(int i = 1; i <= 6 ; i++){
                    if(board[i][0]!=player)break;
                    exact++;
                }
                for(int i = 1 ; i <= 6 ; i++){
                    if(board[0][i]!=player)break;
                    exact++;
                }
            }
            if(board[0][7]==player){
                exact+=1;
                for(int i = 1; i <= 6 ; i++){
                    if(board[i][7]!=player)break;
                    exact++;
                }
                for(int i = 1 ; i <= 6 ; i++){
                    if(board[0][7-i]!=player)break;
                    exact++;
                }
            }
            if(board[7][0]==player){
                exact+=1;
                for(int i = 1; i <= 6 ; i++){
                    if(board[7-i][0]!=player)break;
                    exact++;
                }
                for(int i = 1 ; i <= 6 ; i++){
                    if(board[7][i]!=player)break;
                    exact++;
                }
            }
            if(board[7][7]==player){
                exact+=1;
                for(int i = 1; i <= 6 ; i++){
                    if(board[7-i][7]!=player)break;
                    exact++;
                }
                for(int i = 1 ; i <= 6 ; i++){
                    if(board[7][7-i]!=player)break;
                    exact++;
                }
            }
            return exact;
        }
        int evaluate_Xtrap(){
            int discs1 = 2 , discs2 = 2;
            for(int i = 0 ; i < 8 ; i++){
                if(board[i][i]==get_next_player(player)){
                    discs1=0;
                    break;
                }
            }
            for(int i = 0 ; i < 8 ; i++){
                if(board[i][i]==get_next_player(player)){
                    discs2=0;
                    break;
                }
            }
            return discs1+discs2;
        }
        int evaluate_insider_disc(){
            int discs = 0;
            for(int i =0 ; i < 8 ; i++){
                for(int j = 0 ; j < 8 ; j++){
                    int ok = 1, exact = 1;
                    for(Point dir:directions){
                        Point p = dir+Point(i,j);
                        if(is_spot_on_board(p)&&board[i][j]==EMPTY){
                            ok = 0;
                            break;
                        }
                        if(is_spot_on_board(p)&&board[i][j]==get_next_player(player)){
                            exact = 0;
                            break;
                        }
                    }
                    if(ok)discs++;
                    if(exact)discs++;
                }
            }
            return discs;
        }
        int evaluate_corner(){
            int h = 0;
            if(board[1][0]==player)h+=0.5;  
            if(board[0][1]==player)h+=0.5;   
            if(board[1][1]==player)h++;
            if(board[6][1]==player)h++;
            if(board[6][0]==player)h+=0.5;
            if(board[7][1]==player)h+=0.5;
            if(board[6][7]==player)h+=0.5;
            if(board[7][6]==player)h+=0.5;
            if(board[6][6]==player)h++;
            if(board[0][6]==player)h+=0.5;
            if(board[1][7]==player)h+=0.5;
            if(board[1][6]==player)h++;
            return h;
        }
        void setheuristic(){
            heuristic = 0;
            //if(cur_player==get_next_player(player))return;
            if(winner==player)heuristic+=1;
            heuristic += next_valid_spots.size()*2;
            int exact = evaluate_determined();
            int weak_edge = evaluate_unbalanced_edge();
            int x_trap = evaluate_Xtrap();
            int insider = evaluate_insider_disc();
            int corner = evaluate_corner();
            heuristic -= corner;
            heuristic += x_trap;
            heuristic += insider;
            heuristic -= weak_edge;
            heuristic += exact;
        }
        std::vector<Point> get_valid_spots() const {
            std::vector<Point> valid_spots;
            for (int i = 0; i < SIZE; i++) {
                for (int j = 0; j < SIZE; j++) {
                    Point p = Point(i, j);
                    if (board[i][j] != EMPTY)
                        continue;
                    if (is_spot_valid(p))
                        valid_spots.push_back(p);
                }
            }
            return valid_spots;
        }
        void update(Point &p){
            /*if(!is_spot_valid(p)) {
                winner = get_next_player(cur_player);
                done = true;
                return;
            }*/
            set_disc(p, cur_player);
            disc_count[cur_player]++;
            disc_count[EMPTY]--;
            flip_discs(p);
            // Give control to the other player.
            cur_player = get_next_player(cur_player);
            next_valid_spots = get_valid_spots();
            // Check Win
            if (next_valid_spots.size() == 0) {
                cur_player = get_next_player(cur_player);
                next_valid_spots = get_valid_spots();
                if (next_valid_spots.size() == 0) {
                    // Game ends
                    done = true;
                    int white_discs = disc_count[WHITE];
                    int black_discs = disc_count[BLACK];
                    if (white_discs == black_discs) winner = EMPTY;
                    else if (black_discs > white_discs) winner = BLACK;
                    else winner = WHITE;
                }
            }
            setheuristic();
        }
};

int minimax(State& s, int depth, int max_player, int alpha, int beta){
    if(depth==0||s.done)return s.heuristic;
    if(max_player){
        int bestvalue = INT_MIN;
        for(Point nxt : s.next_valid_spots){
            State next_state = s;
            next_state.update(nxt);
            if(next_state.done&&next_state.cur_player!=player)next_state.heuristic = s.heuristic;
            bestvalue = std::max(bestvalue, minimax(next_state, depth-1, 0, alpha, beta));
            alpha = std::max(bestvalue, alpha);
            if(beta <= alpha)
                break;
        }
        return bestvalue;
    }
    else{
        int bestvalue = INT_MAX;
        for(Point nxt : s.next_valid_spots){
            State next_state = s;
            next_state.update(nxt);
            if(next_state.done&&next_state.cur_player!=player)next_state.heuristic = s.heuristic;
            bestvalue = std::min(bestvalue, minimax(next_state,depth-1, 1, alpha, beta));
            beta = std::min(bestvalue, beta);
            if(beta <= alpha)
                break;
        }
        return bestvalue;
    }
}
void read_board(std::ifstream& fin) {
    for(int i =0 ; i < 3 ; i++)cur_disc_count[i] = 0;
    fin >> player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> cur_board[i][j];
            if(cur_board[i][j]==0){
                cur_disc_count[0]++;
            }
            else if(cur_board[i][j]==1){
                cur_disc_count[1]++;
            }
            else if(cur_board[i][j]==2){
                cur_disc_count[2]++;
            }
        }
    }
}

void read_valid_spots(std::ifstream& fin) {
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        Point cur(x,y);
        if(cur==Point(1,1)||cur==Point(1,6)||cur==Point(6,1)||cur==Point(6,6)){
            cur_x_spots.push_back({x,y});
        }
        else 
            cur_next_valid_spots.push_back(cur);
    }
}
void write_valid_spot(std::ofstream& fout) {
    int n_valid_spots = cur_next_valid_spots.size();
    int bestspot = -1, ok = 0, corner = 0;
    int bestvalue=INT_MIN, best_determined_value = 0, turn_discs = 0, best_corner_value = 0;
    State cur_state;
    for(int i = 0 ; i < n_valid_spots; i++){
        State tmp_state;
        Point nextspot = cur_next_valid_spots[i];
        tmp_state.update(nextspot);
        int add = tmp_state.disc_count[player]-cur_state.disc_count[player];
        if((nextspot==Point(1,0)||nextspot==Point(0,1))&&tmp_state.board[0][0]!=player){
            cur_x_spots.push_back(nextspot);
            continue;
        }
        if((nextspot==Point(1,7)||nextspot==Point(0,6))&&tmp_state.board[0][7]!=player){
            cur_x_spots.push_back(nextspot);
            continue;
        }
        if((nextspot==Point(6,0)||nextspot==Point(7,1))&&tmp_state.board[7][0]!=player){
            cur_x_spots.push_back(nextspot);
            continue;
        }
        if((nextspot==Point(7,6)||nextspot==Point(6,7))&&tmp_state.board[7][7]!=player){
            cur_x_spots.push_back(nextspot);
            continue;
        }
        if(nextspot==Point(0,0)||nextspot==Point(7,0)||nextspot==Point(0,7)||nextspot==Point(7,7)){
            if(add>best_corner_value){
                bestspot = i;
                best_corner_value = add;
            }  
            corner = 1;
        }
        if(corner)continue;
        int determined_value = tmp_state.evaluate_determined()-cur_state.evaluate_determined();
        if(determined_value>best_determined_value&&add>turn_discs){
            ok = 1;
            bestspot = i;
            best_determined_value = determined_value;
            turn_discs = add;
        }
        if(ok)continue;
        int cur_value = minimax(tmp_state, 5, 0, INT_MIN, INT_MAX);
        if(bestvalue<=cur_value){
            bestvalue = cur_value;
            bestspot = i;
        }
    }
    Point p ;
    if(bestspot!=-1)p = cur_next_valid_spots[bestspot];
    else{
        int x_spot = cur_x_spots.size();
        int better = 0, worse_spot = -1;
        for(int i = 0 ; i < x_spot ; i++){
            State tmp_state;
            tmp_state.update(cur_x_spots[i]);
            int cur_value = minimax(tmp_state, 5, 0, INT_MIN, INT_MAX);
            if(better<=cur_value){
                worse_spot = i;
                better = cur_value;
            }
        }
        if(worse_spot!=-1)p = cur_x_spots[worse_spot];
    }
    fout << p.x << " " << p.y << std::endl;
    fout.flush();
}

int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}
