import React, { useState, useEffect } from 'react';
import Chart from "chart.js/auto";
import { Pie } from 'react-chartjs-2';

const MemoryChart = () => {
    const API_URL = import.meta.env.VITE_API_URL
    const ENDPOINT_MEMORY = import.meta.env.VITE_ENDPOINT_MEMORY
    const [memoryData, setMemoryData] = useState([25, 25, 25, 25])

    const updateMemoryData = async () => {
        const requestOptions = {
            method: 'GET',
        }
        const response = await fetch(API_URL + ENDPOINT_MEMORY, requestOptions);
        const data = await response.json();
        setMemoryData([data.total_memory, data.free_memory, data.used_memory, data.cached_memory])
    }

    useEffect(() => {
        // Esta función se ejecutará cada 1000 milisegundos (1 segundo)
        const intervalId = setInterval(() => {
            updateMemoryData();
        }, 1000);

        // Limpieza del intervalo cuando el componente se desmonta o actualiza
        return () => clearInterval(intervalId);
    }, []);

    const data = {
        labels: ['Total', 'Libre', 'Utilizada', 'Caché'],
        datasets: [
            {
                label: 'Uso de memoria',
                data: memoryData,
                backgroundColor: ['rgba(96, 149, 255, 0.2)', 'rgba(160, 255, 125, 0.2)', 'rgba(255, 108, 108, 0.2)', 'rgba(255, 224, 96, 0.2)'],
                hoverBackgroundColor: ['rgba(96, 149, 255, 1)', 'rgba(160, 255, 125, 1)', 'rgba(255, 108, 108, 1)', 'rgba(255, 224, 96, 1)'],
            },
        ],
    };

    return (
        <>
            <div className='container mx-auto py-5'>
                <h1 className="text-center text-5xl my-8">Memoria RAM</h1>
                <div className="grid h-auto flex-grow card bg-base-300 rounded-box place-items-center py-7">
                    <h2 className="text-center text-3xl mb-2">Uso de RAM (KB)</h2>
                    <div className="py-2">
                        <Pie data={data} />
                    </div>
                </div>

            </div>
        </>
    );
};

export default MemoryChart;