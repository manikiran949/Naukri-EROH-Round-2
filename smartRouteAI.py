import numpy as np
import pandas as pd
import heapq
from sklearn.preprocessing import MinMaxScaler
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import LSTM, Dense
from tensorflow.keras.callbacks import EarlyStopping

# Load and preprocess traffic data
df = pd.read_csv('traffic_data.csv', parse_dates=['timestamp'])
df.sort_values('timestamp', inplace=True)
data = df['congestion'].values.reshape(-1, 1)
scaler = MinMaxScaler()
data_scaled = scaler.fit_transform(data)

def create_sequences(data, seq_length):
    X, y = [], []
    for i in range(len(data) - seq_length):
        X.append(data[i:i+seq_length])
        y.append(data[i+seq_length])
    return np.array(X), np.array(y)

seq_length = 10
X, y = create_sequences(data_scaled, seq_length)
split = int(0.8 * len(X))
X_train, y_train = X[:split], y[:split]
X_test, y_test = X[split:], y[split:]

model = Sequential([LSTM(50, activation='relu', input_shape=(seq_length, 1)), Dense(1)])
model.compile(optimizer='adam', loss='mse')
early_stopping = EarlyStopping(monitor='val_loss', patience=5)
model.fit(X_train, y_train, epochs=100, batch_size=32, validation_split=0.2, callbacks=[early_stopping], verbose=1)

def predict_congestion_from_sequence(seq):
    seq = np.array(seq).reshape(-1, 1)
    seq_scaled = scaler.transform(seq)
    seq_scaled = seq_scaled.reshape(1, seq_length, 1)
    pred_scaled = model.predict(seq_scaled)
    pred = scaler.inverse_transform(pred_scaled)
    return pred[0][0]

# In a real scenario, replace this with actual historical data retrieval per edge (u,v)
def get_historical_data(u, v):
    return np.random.rand(seq_length)

graph = {
    'A': [('B', 5), ('C', 10)],
    'B': [('C', 3), ('D', 2)],
    'C': [('D', 1)],
    'D': []
}

def a_star(graph, start, goal):
    queue = []
    heapq.heappush(queue, (0, start, [start]))
    visited = set()
    while queue:
        cost, current, path = heapq.heappop(queue)
        if current == goal:
            return path, cost
        if current in visited:
            continue
        visited.add(current)
        for neighbor, base_time in graph.get(current, []):
            hist_seq = get_historical_data(current, neighbor)
            congestion = predict_congestion_from_sequence(hist_seq)
            travel_time = base_time * (1 + congestion)
            new_cost = cost + travel_time
            new_path = path + [neighbor]
            heapq.heappush(queue, (new_cost, neighbor, new_path))
    return None, float('inf')

path, total_time = a_star(graph, 'A', 'D')
print("Optimal path:", path)
print("Total travel time:", total_time)
