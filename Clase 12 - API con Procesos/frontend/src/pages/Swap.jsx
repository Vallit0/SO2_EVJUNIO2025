import React, { useState, useEffect } from 'react';
import Chart from "chart.js/auto";
import { Line } from 'react-chartjs-2';

const SwapChart = () => {
    const API_URL = import.meta.env.VITE_API_URL
    const ENDPOINT_SWAP = import.meta.env.VITE_ENDPOINT_SWAP
    const ENDPOINT_MEMORY = import.meta.env.VITE_ENDPOINT_MEMORY
    
    /**
     * timestamp
     * x -> formatted timestamp
     * y -> value
     */
    const [usedSwapData, setUsedSwapData] = useState([]);
    const [freeSwapData, setFreeSwapData] = useState([]);
    const [usedMemoryData, setUsedMemoryData] = useState([]);
    const [freeMemoryData, setFreeMemoryData] = useState([]);

    const getTimestamp = () => {
        return new Date().getTime();
    }

    const formatTimestamp = (timestamp) => {
        const date = new Date(timestamp);
    
        const hours = String(date.getUTCHours()).padStart(2, '0');
        const minutes = String(date.getUTCMinutes()).padStart(2, '0');
        const seconds = String(date.getUTCSeconds()).padStart(2, '0');
    
        return `${hours}:${minutes}:${seconds}`;
    }

    const getRecents = (data) => {
        return data.slice(Math.max(data.length - 8, 0));
    }

    const updateMemoryData = async () => {
        const requestOptions = {
            method: 'GET',
        }
        const response = await fetch(API_URL + ENDPOINT_MEMORY, requestOptions);
        const data = await response.json();
        const timestamp = getTimestamp();

        let fmtFreeMemData = {
            timestamp: timestamp,
            x: formatTimestamp(timestamp),
            y: data.used_memory,
        }

        let fmtUsedMemData = {
            timestamp: timestamp,
            x: formatTimestamp(timestamp),
            y: data.free_memory,
        }

        const arrayFreeMem = usedMemoryData;
        const arrayUsedMem = freeMemoryData;

        arrayFreeMem.push(fmtFreeMemData);
        arrayUsedMem.push(fmtUsedMemData);

        arrayFreeMem.sort((a, b) => a.timestamp - b.timestamp);
        arrayUsedMem.sort((a, b) => a.timestamp - b.timestamp);

        const recentFreeMem = getRecents(arrayFreeMem);
        const recentUsedMem = getRecents(arrayUsedMem);

        setFreeMemoryData(recentFreeMem);
        setUsedMemoryData(recentUsedMem);
    }
    
    const updateSwapData = async () => {
        const requestOptions = {
            method: 'GET',
        }
        const response = await fetch(API_URL + ENDPOINT_SWAP, requestOptions);
        const data = await response.json();
        const timestamp = getTimestamp();

        let fmtFreeSwapData = {
            timestamp: timestamp,
            x: formatTimestamp(timestamp),
            y: data.free_mem,
        }

        let fmtUsedSwapData = {
            timestamp: timestamp,
            x: formatTimestamp(timestamp),
            y: data.used_mem,
        }

        const arrayFreeSwap = freeSwapData;
        const arrayUsedSwap = usedSwapData;

        arrayFreeSwap.push(fmtFreeSwapData);
        arrayUsedSwap.push(fmtUsedSwapData);

        arrayFreeSwap.sort((a, b) => a.timestamp - b.timestamp);
        arrayUsedSwap.sort((a, b) => a.timestamp - b.timestamp);

        const recentFreeSwap = getRecents(arrayFreeSwap);
        const recentUsedSwap = getRecents(arrayUsedSwap);

        setFreeSwapData(recentFreeSwap);
        setUsedSwapData(recentUsedSwap);
    }

    useEffect(() => {
        const intervalId = setInterval(() => {
            updateSwapData();
            updateMemoryData();
        }, 2000);

        return () => clearInterval(intervalId);
    }, []);

    const swapData = {
        labels: freeSwapData.map(data => data.x),
        datasets: [
            {
                label: 'SWAP libre',
                data: freeSwapData.map(data => data.y),
                borderColor: 'rgba(96, 149, 255, 1)',
                borderWidth: 2,
                backgroundColor: 'rgba(96, 149, 255, 0.2)',
                yAxisID: 'y',
                fill: false,
            },
            {
                label: 'SWAP utilizado',
                data: usedSwapData.map(data => data.y),
                borderWidth: 2,
                borderColor: 'rgba(255, 108, 108, 1)',
                backgroundColor: 'rgba(255, 108, 108, 0.2)',
                yAxisID: 'y1',
                fill: false,
            },
        ],
    };

    const memoryData = {
        labels: freeMemoryData.map(data => data.x),
        datasets: [
            {
                label: 'Memoria libre',
                data: freeMemoryData.map(data => data.y),
                borderColor: 'rgba(96, 149, 255, 1)',
                borderWidth: 2,
                backgroundColor: 'rgba(96, 149, 255, 0.2)',
                yAxisID: 'y',
                fill: false,
            },
            {
                label: 'Memoria utilizada',
                data: usedMemoryData.map(data => data.y),
                borderWidth: 2,
                borderColor: 'rgba(255, 108, 108, 1)',
                backgroundColor: 'rgba(255, 108, 108, 0.2)',
                yAxisID: 'y1',
                fill: false,
            },
        ],
    };


    const options = {
        scales: {
            y: {
              type: 'linear',
              display: true,
              position: 'left',
            },
            y1: {
              type: 'linear',
              display: true,
              position: 'right',
      
              // grid line settings
              grid: {
                drawOnChartArea: false, // only want the grid lines for one axis to show up
              },
            },
        },
    };

    return (
        <div className='container mx-auto py-5 h-full'>
            <h1 className="text-center text-5xl my-8">Grafica de lineas para memoria SWAP y fisica</h1>
            <div className="flex flex-col w-full">
                <div className="grid h-auto flex-grow card bg-base-300 rounded-box place-items-center py-5">
                    <h2 className="text-center text-3xl mb-2">SWAP: Libre y utilizada (KB)</h2>
                    <div className="py-5 px-5 w-10/12">
                        <Line data={swapData} options={options} height={"25rem"} width={"100%"} />
                    </div>
                </div>
                <div className="divider"></div>
                <div className="grid h-auto flex-grow card bg-base-300 rounded-box place-items-center py-5">
                    <h2 className="text-center text-3xl mb-2">Memoria fisica: Libre y utilizada (KB)</h2>
                    <div className="py-5 px-5 w-10/12">
                        <Line data={memoryData} options={options} height={"25rem"} width={"100%"} />
                    </div>
                </div>
            </div>
        </div>
    );
}

export default SwapChart;