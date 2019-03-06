#include <iostream>
#include "QProgToQRunes.h"
#include "QPanda.h"
#include "QVec.h"
using QGATE_SPACE::angleParameter;
using namespace std;
USING_QPANDA
QProgToQRunes::QProgToQRunes()
{
    m_gatetype.insert(pair<int, string>(PAULI_X_GATE, "X"));
    m_gatetype.insert(pair<int, string>(PAULI_Y_GATE, "Y"));
    m_gatetype.insert(pair<int, string>(PAULI_Z_GATE, "Z"));

    m_gatetype.insert(pair<int, string>(X_HALF_PI, "X1"));
    m_gatetype.insert(pair<int, string>(Y_HALF_PI, "Y1"));
    m_gatetype.insert(pair<int, string>(Z_HALF_PI, "Z1"));

    m_gatetype.insert(pair<int, string>(HADAMARD_GATE, "H"));
    m_gatetype.insert(pair<int, string>(T_GATE,        "T"));
    m_gatetype.insert(pair<int, string>(S_GATE,        "S"));

    m_gatetype.insert(pair<int, string>(RX_GATE, "RX"));
    m_gatetype.insert(pair<int, string>(RY_GATE, "RY"));
    m_gatetype.insert(pair<int, string>(RZ_GATE, "RZ"));

    m_gatetype.insert(pair<int, string>(U1_GATE, "U1"));
    m_gatetype.insert(pair<int, string>(U2_GATE, "U2"));
    m_gatetype.insert(pair<int, string>(U3_GATE, "U3"));
    m_gatetype.insert(pair<int, string>(U4_GATE, "U4"));

    m_gatetype.insert(pair<int, string>(CU_GATE, "CU"));
    m_gatetype.insert(pair<int, string>(CNOT_GATE, "CNOT"));
    m_gatetype.insert(pair<int, string>(CZ_GATE, "CZ"));
    m_gatetype.insert(pair<int, string>(CPHASE_GATE, "CPHASE"));
    m_gatetype.insert(pair<int, string>(ISWAP_GATE, "ISWAP"));
    m_gatetype.insert(pair<int, string>(SQISWAP_GATE, "SQISWAP"));
    m_QRunes.clear();
}


QProgToQRunes::~QProgToQRunes()
{
}


void QProgToQRunes::qProgToQRunes(AbstractQuantumProgram * pQpro)
{
    if (nullptr == pQpro)
    {
        QCERR("pQPro is null");
        throw invalid_argument("pQPro is null");
    }
    m_QRunes.emplace_back("QINIT " + to_string(getAllocateQubitNum()));
    m_QRunes.emplace_back("CREG "  + to_string(getAllocateCMem()));

    QProgToQRunes::transformQProg(pQpro);

}


void QProgToQRunes::transformQProg(AbstractQGateNode * pQGata)
{
    if (nullptr == pQGata || nullptr == pQGata->getQGate())
    {
        QCERR("pQGata is null");
        throw invalid_argument("pQGata is null");
    }

    int gate_type = pQGata->getQGate()->getGateType();

    QVec qubits_vector;
    QVec ctr_qubits_vector;

    std::string all_ctr_qubits;
    pQGata->getQuBitVector(qubits_vector);
    pQGata->getControlVector(ctr_qubits_vector);

    if (pQGata->isDagger())
    {
        m_QRunes.emplace_back("DAGGER");
    }
    if (!ctr_qubits_vector.empty())
    {
        for (auto val : ctr_qubits_vector)
        {
            all_ctr_qubits = all_ctr_qubits + to_string(val->getPhysicalQubitPtr()->getQubitAddr()) + ",";
        }
        all_ctr_qubits = all_ctr_qubits.substr(0, all_ctr_qubits.length() - 1);
        m_QRunes.emplace_back("CONTROL " + all_ctr_qubits);
    }

    auto iter = m_gatetype.find(gate_type);
    if (iter == m_gatetype.end())
    {
        QCERR("unknow error");
        throw runtime_error("unknow error");
    }

    string item = iter->second;
    string first_qubit = to_string(qubits_vector.front()->getPhysicalQubitPtr()->getQubitAddr());
    string all_qubits;

    for (auto _val : qubits_vector)
    {
        all_qubits = all_qubits + to_string(_val->getPhysicalQubitPtr()->getQubitAddr()) + ",";
    }

    all_qubits = all_qubits.substr(0, all_qubits.length() - 1);
    switch (iter->first)
    {
    case PAULI_X_GATE:
    case PAULI_Y_GATE:
    case PAULI_Z_GATE:
    case X_HALF_PI:
    case Y_HALF_PI:
    case Z_HALF_PI:
    case HADAMARD_GATE:
    case T_GATE:
    case S_GATE: 
        {
            m_QRunes.emplace_back(item + " " + first_qubit);
        }
        break;

    case U1_GATE:
    case RX_GATE:
    case RY_GATE:
    case RZ_GATE:
        {
            angleParameter * gate_parameter = dynamic_cast<angleParameter *>(pQGata->getQGate());
            if (nullptr == gate_parameter)
            {
                QCERR("gate_parameter is null");
                throw runtime_error("gate_parameter is null");
            }
            string  gate_angle = to_string(gate_parameter->getParameter());
            m_QRunes.emplace_back(item + " " + first_qubit + ","+"\"" + gate_angle+"\"");
        }
        break;

    case CNOT_GATE:
    case CZ_GATE:
    case ISWAP_GATE:
    case SQISWAP_GATE:
        {
            m_QRunes.emplace_back(item + " " + all_qubits);
        }
        break;

    case CPHASE_GATE: 
        {
            angleParameter * gate_parameter = dynamic_cast<angleParameter *>(pQGata->getQGate());
            if (nullptr == gate_parameter)
            {
                QCERR("gate_parameter is null");
                throw runtime_error("gate_parameter is null");
            }
            string  gate_theta = to_string(gate_parameter->getParameter());
            m_QRunes.emplace_back(item + " " + all_qubits + "," + gate_theta);
        }
        break;

    case CU_GATE: 
       { 
            QuantumGate * gate_parameter = dynamic_cast<QuantumGate *>(pQGata->getQGate());
            if (nullptr == gate_parameter)
            {
                QCERR("gate_parameter is null");
                throw runtime_error("gate_parameter is null");
            }
            string gate_four_theta = to_string(gate_parameter->getAlpha()) + ',' + 
                                     to_string(gate_parameter->getBeta())  + ',' +
                                     to_string(gate_parameter->getDelta()) + ',' + 
                                     to_string(gate_parameter->getGamma());
            m_QRunes.emplace_back(item + " " + all_qubits + "," + gate_four_theta);
       }
       break;

    default:m_QRunes.emplace_back("UnSupported GateNode");
    }
    if (!ctr_qubits_vector.empty())
    {
        m_QRunes.emplace_back("ENCONTROL " + all_ctr_qubits);
    }
    if (pQGata->isDagger())
    {
        m_QRunes.emplace_back("ENDAGGER");
    }
}


void QProgToQRunes::transformQProg(AbstractQuantumMeasure *pMeasure)
{
    if (nullptr == pMeasure)
    {
        QCERR("pMeasure is null");
        throw invalid_argument("pMeasure is null");
    }

    PhysicalQubit* qbitPtr = pMeasure->getQuBit()->getPhysicalQubitPtr();

    if (nullptr == qbitPtr)
    {
        QCERR("pMeasure is null");
        throw runtime_error("pMeasure is null");
    }

    std::string tar_qubit = to_string(qbitPtr->getQubitAddr());
    std::string creg_name = pMeasure->getCBit()->getName();

    creg_name = creg_name.substr(1);
    m_QRunes.emplace_back("MEASURE " + tar_qubit + ",$" + creg_name);
}


void QProgToQRunes::transformQProg(AbstractQuantumProgram *pQPro)
{
    if (nullptr == pQPro)
    {
        QCERR("pQPro is null");
        throw runtime_error("pQPro is null");
    }

    for (auto aiter = pQPro->getFirstNodeIter(); aiter != pQPro->getEndNodeIter(); aiter++)
    {
        QNode * pNode = (*aiter).get();
        transformQProg(pNode);
    }
}


void QProgToQRunes::transformQProg(QNode * pNode)
{
    if (nullptr == pNode)
    {
        QCERR("pNode is null");
        throw runtime_error("pNode is null");
    }

    int type = pNode->getNodeType();

    switch (type)
    {
    case NodeType::GATE_NODE:
        transformQProg(dynamic_cast<AbstractQGateNode *>(pNode));
        break;

    case NodeType::CIRCUIT_NODE:
        transformQProg(dynamic_cast<AbstractQuantumCircuit *>(pNode));
        break;

    case NodeType::PROG_NODE:
        transformQProg(dynamic_cast<AbstractQuantumProgram *>(pNode)); 
        break;

    case NodeType::QIF_START_NODE:
    case NodeType::WHILE_START_NODE:
        transformQProg(dynamic_cast<AbstractControlFlowNode *>(pNode));
        break;

    case NodeType::MEASURE_GATE:
        transformQProg(dynamic_cast<AbstractQuantumMeasure *>(pNode));
        break;

    case NodeType::NODE_UNDEFINED:
    default:m_QRunes.emplace_back("UnSupported ProgNode");
        break;
    }
}


static void traversalInOrderPCtr(const CExpr* pCtrFlow,string &ctr_statement)
{
    if (nullptr != pCtrFlow)
    {
        traversalInOrderPCtr(pCtrFlow->getLeftExpr(), ctr_statement);
        ctr_statement = ctr_statement + pCtrFlow->getName();
        traversalInOrderPCtr(pCtrFlow->getRightExpr(), ctr_statement);
    }
}


void QProgToQRunes::transformQProg(AbstractControlFlowNode * pCtrFlow)
{
    if (nullptr == pCtrFlow)
    {
        QCERR("pCtrFlow is null");
        throw runtime_error("pCtrFlow is null");
    }

    QNode *pNode = dynamic_cast<QNode *>(pCtrFlow);
    if (nullptr == pNode)
    {
        QCERR("pNode is null");
        throw runtime_error("pNode is null");
    }

    int node_type = pNode->getNodeType();

    switch (node_type)
    {
    case NodeType::WHILE_START_NODE:
        {
            string exper;
            auto expr = pCtrFlow->getCExpr()->getExprPtr().get();

            traversalInOrderPCtr(expr, exper);
            m_QRunes.emplace_back("QWHILE " + exper);

            QNode *truth_branch_node = pCtrFlow->getTrueBranch();
            if (nullptr != truth_branch_node)
            {
                transformQProg(truth_branch_node);
            }

            m_QRunes.emplace_back("ENDQWHILE");
        }
        break;

    case NodeType::QIF_START_NODE:
        {
            string exper;

            auto expr = pCtrFlow->getCExpr()->getExprPtr().get();

            traversalInOrderPCtr(expr, exper);
            m_QRunes.emplace_back("QIF "+exper);

            QNode * truth_branch_node = pCtrFlow->getTrueBranch();
            if (nullptr != truth_branch_node)
            {
                transformQProg(truth_branch_node);
            }
            m_QRunes.emplace_back("ELSE");

            QNode *false_branch_node = pCtrFlow->getFalseBranch();
            if (nullptr != false_branch_node)
            {
                transformQProg(false_branch_node);
            }
            m_QRunes.emplace_back("ENDQIF");
        }
        break;

    }
}


void QProgToQRunes::transformQProg(AbstractQuantumCircuit * pCircuit)
{
    if (nullptr == pCircuit)
    {
        QCERR("pCircuit is null");
        throw runtime_error("pCircuit is null");
    }
    if (pCircuit->isDagger())
    {
        m_QRunes.emplace_back("DAGGER");
    }

    QVec circuit_ctr_qubits;

    string all_ctr_qubits;

    pCircuit->getControlVector(circuit_ctr_qubits);
    if (!circuit_ctr_qubits.empty())
    {
        for (auto val : circuit_ctr_qubits)
        {
            all_ctr_qubits = all_ctr_qubits + to_string(val->getPhysicalQubitPtr()->getQubitAddr()) + ",";
        }
        all_ctr_qubits = all_ctr_qubits.substr(0, all_ctr_qubits.length() - 1);
        m_QRunes.emplace_back("CONTROL " + all_ctr_qubits);
    }
    for (auto aiter = pCircuit->getFirstNodeIter(); aiter != pCircuit->getEndNodeIter(); aiter++)
    {
        QNode * pNode = (*aiter).get();
        transformQProg(pNode);
    }
    if (!circuit_ctr_qubits.empty())
    {
        m_QRunes.emplace_back("ENCONTROL " + all_ctr_qubits);
    }
    if (pCircuit->isDagger())
    {
        m_QRunes.emplace_back("ENDAGGER");
    }

}


string QProgToQRunes::insturctionsQRunes()
{
    string instructions;

    for (auto &instruct_out : m_QRunes)
    {
        instructions.append(instruct_out).append("\n");
    }
    instructions.erase(instructions.size() - 1);

    return instructions;
}


ostream & QPanda::operator<<(ostream & out, const QProgToQRunes &qrunes_prog)
{
    for (auto instruct_out : qrunes_prog.m_QRunes)
    {
        out << instruct_out << "\n";
    }

    return out;
}